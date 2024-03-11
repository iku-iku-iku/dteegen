#include "llm.h"

#include <TEE-Capability/common.h>

#include <cstdint>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>

int cnt = 0;
int max_cnt = 0;
std::unordered_map<void*, int> g_alloc_map;
static bool flag = false;
#if 1
#define record(x, y)
#define unrecord(x)
#else
static void record(void* ptr, size_t size)
{
    if (flag) return;
    flag = true;
    g_alloc_map[ptr] = size;
    flag = false;
    cnt += size;
    max_cnt = std::max(cnt, max_cnt);
}
static void unrecord(void* ptr)
{
    if (flag) return;
    flag = true;
    cnt -= g_alloc_map[ptr];
    flag = false;
    g_alloc_map[ptr] = 0;
}
#endif
#include "../insecure/file_stub.h"
#include "common/common.h"
#include "llama.h"

#ifdef USE_TLSF
#include "tlsf.h"

tlsf_t g_tlsf_pool;
#endif
#define MAX_POXIS_MEMALIGN 10
static void* ptrs[MAX_POXIS_MEMALIGN];
static size_t offset[MAX_POXIS_MEMALIGN];
extern "C"
{
    typedef size_t (*fread_type)(void* __restrict ptr, size_t size, size_t n,
                                 FILE* __restrict stream);
    typedef size_t (*fwrite_type)(const void* __restrict __ptr, size_t __size,
                                  size_t __n, FILE* __restrict __s);
    typedef FILE* (*fopen_type)(const char* __restrict __filename,
                                const char* __restrict __modes);
    typedef int (*fclose_type)(FILE* __stream);
    typedef int (*fseek_type)(FILE* __stream, long int __off, int __whence);
    typedef long int (*ftell_type)(FILE* __stream);

    struct io_helper
    {
        fread_type fread;
        fwrite_type fwrite;
        fopen_type fopen;
        fclose_type fclose;
        fseek_type fseek;
        ftell_type ftell;
    };
    io_helper g_io_helper;
}

#define BUF_LEN 1024

int run(const char* prompt)
{
    gpt_params params;
    params.n_threads = 1;
    params.n_threads_batch = 1;
    params.prompt = prompt;
    params.model = "qwen-model";
    params.seed = 691;
    eapp_print("OK\n");
    // log_disable();
    eapp_print("OK\n");

    llama_sampling_params& sparams = params.sparams;
    std::mt19937 rng(params.seed);
    malloc(31);
    eapp_print("OK\n");

    llama_backend_init();
    eapp_print("OK\n");
    // llama_numa_init(params.numa);

    llama_model* model;
    llama_context* ctx;
    llama_context* ctx_guidance = NULL;

    std::tie(model, ctx) = llama_init_from_gpt_params(params);
    eapp_print("OK\n");
    if (model == NULL) {
        eapp_print("%s: error: unable to load model\n", __func__);
        return 1;
    }

    const int n_ctx_train = llama_n_ctx_train(model);
    const int n_ctx = llama_n_ctx(ctx);

    const bool add_bos = llama_should_add_bos_token(model);

    std::vector<llama_token> embd_inp;
    embd_inp = ::llama_tokenize(ctx, params.prompt, add_bos, true);

    if ((int)embd_inp.size() > n_ctx - 4) {
        eapp_print("%s: error: prompt is too long (%d tokens, max %d)\n",
                   __func__, (int)embd_inp.size(), n_ctx - 4);
        return 1;
    }

    // number of tokens to keep when resetting context
    if (params.n_keep < 0 || params.n_keep > (int)embd_inp.size() ||
        params.instruct || params.chatml) {
        params.n_keep = (int)embd_inp.size();
    }
    else {
        params.n_keep += add_bos;  // always keep the BOS token
    }

    int n_past = 0;
    int n_remain = params.n_predict;
    int n_consumed = 0;

    std::vector<llama_token> embd;
    std::vector<llama_token> embd_guidance;

    struct llama_sampling_context* ctx_sampling = llama_sampling_init(sparams);

    while (n_remain != 0) {
        // predict
        if (!embd.empty()) {
            eapp_print("");
            int max_embd_size = n_ctx - 4;

            // Ensure the input doesn't exceed the context size by truncating
            // embd if necessary.
            if ((int)embd.size() > max_embd_size) {
                const int skipped_tokens = (int)embd.size() - max_embd_size;
                embd.resize(max_embd_size);

                fflush(stdout);
            }

            for (int i = 0; i < (int)embd.size(); i += params.n_batch) {
                int n_eval = (int)embd.size() - i;
                if (n_eval > params.n_batch) {
                    n_eval = params.n_batch;
                }

                if (llama_decode(ctx, llama_batch_get_one(&embd[i], n_eval,
                                                          n_past, 0))) {
                    eapp_print("%s : failed to eval\n", __func__);
                    return 1;
                }

                n_past += n_eval;
            }
        }

        embd.clear();
        embd_guidance.clear();

        eapp_print("");
        if ((int)embd_inp.size() <= n_consumed) {
            const llama_token id =
                llama_sampling_sample(ctx_sampling, ctx, ctx_guidance);

            llama_sampling_accept(ctx_sampling, ctx, id, true);
            embd.push_back(id);

            // decrement remaining sampling budget
            --n_remain;
        }
        else {
            // some user input remains from prompt or interaction, forward it to
            // processing
            while ((int)embd_inp.size() > n_consumed) {
                embd.push_back(embd_inp[n_consumed]);

                llama_sampling_accept(ctx_sampling, ctx, embd_inp[n_consumed],
                                      false);

                ++n_consumed;
                if ((int)embd.size() >= params.n_batch) {
                    break;
                }
            }
        }
        eapp_print("");

        // display text
        for (auto id : embd) {
            const std::string token_str = llama_token_to_piece(ctx, id);
            eapp_print("%s", token_str.c_str());
        }
        fflush(stdout);

        // end of text token
        if (!embd.empty() && embd.back() == llama_token_eos(model)) {
            eapp_print(" [end of text]\n");
            break;
        }
    }
    llama_free(ctx);
    llama_free_model(model);

    llama_sampling_free(ctx_sampling);
    llama_backend_free();

    return 0;
}

extern "C"
{
    static size_t buf_pos = 0;
    static size_t buf_end = 0;
    FILE* my_fopen(const char* __restrict __filename,
                   const char* __restrict __modes)
    {
        buf_pos = 0;
        buf_end = 0;
        __filename = "qwen-model";
        eapp_print("FILENAME: %s\n", __filename);
        return (FILE*)(open_file((char*)__filename, strlen(__filename) + 1));
    }

    size_t my_fread(void* __restrict ptr, size_t size, size_t n,
                    FILE* __restrict stream)
    {
        static char buf[4096 * 10];
        // eapp_print("------------BEGIN ERAD %d x %d-----------\n", (int)size,
        // (int)n, (void*)ptr);
        if (stream == NULL) {
            eapp_print("stream is NULL\n");
            return 0;
        }
        auto p = (char*)ptr;
        int t0 = 0;
        size_t remain = size * n;
        // read_file((size_t)stream, 1, remain, (char*)ptr, remain);
        while (remain > 0) {
            if (buf_pos >= buf_end) {
                // eapp_print("OCALL BEGIN\n");
                buf_end = read_file((size_t)stream, 1, sizeof(buf), (char*)buf,
                                    sizeof(buf));
                // eapp_print("OCALL END, buf_end: %lu, %d\n", buf_end,
                //            (int)buf[0]);
                buf_pos = 0;
            }
            int read_end = std::min(buf_end, buf_pos + remain);
            int read_len = read_end - buf_pos;
            // eapp_print("from: %p to: %p len: %d\n", buf + buf_pos, p,
            // read_len);

            memcpy(p, buf + buf_pos, read_len);
            if (p == ptr) t0 = ((char*)p)[0];
            // break;

            p += read_len;
            buf_pos += read_len;
            remain -= read_len;
        }

        // eapp_print("------------END ERAD-----------\n");
        return n;
    }

    size_t my_fwrite(const void* __restrict __ptr, size_t __size, size_t __n,
                     FILE* __restrict __s)
    {
        eapp_print("WRITE: %lu x %lu at %p\n", __size, __n, __ptr);
        eapp_print("fwrite is not supported now.\n");
        return 0;
    }

    int my_fclose(FILE* __stream)
    {
        eapp_print("CLOSE\n");
        return close_file((size_t)__stream);
    }

    int my_fseek(FILE* __stream, long int __offset, int __whence)
    {
        eapp_print("SEEK\n");
        return seek_file((size_t)__stream, __offset, __whence);
    }

    long int my_ftell(FILE* __stream)
    {
        eapp_print("TELL\n");
        return tell_file((size_t)__stream);
    }

    typedef void* (*malloc_type)(size_t __size);
    malloc_type g_my_malloc;
    typedef int (*posix_memalign_type)(void** __memptr, size_t __alignment,
                                       size_t __size);
    posix_memalign_type g_my_posix_memalign;
    typedef void (*free_type)(void* __ptr);
    free_type g_my_free;
    typedef void* (*calloc_type)(size_t, size_t);
    calloc_type g_my_calloc;
}

char* g_mem = 0;
#define HEAP_LEN (450 * 1024 * 1024)
size_t g_pos;
int free_cnt = 0;

// offset_to_next data offset_of_data
// two empty block will merge

// void* my_malloc_lite(size_t size)
// {
//     // static size_t total = 0;
//     // total += size;
//     // return malloc(size);
//     static char g_stk[1024 * 1024 * 400];
//     static int g_top = 0;
//
//     // eapp_print("MALLOC %d\n", (int)size);
//     // char* top = g_mem + g_pos;
//
//     auto res = g_stk + g_top;
//     g_top += size;
//     // eapp_print("MALLOC: %d\n", (int)size);
//     // if (g_pos >= HEAP_LEN) {
//     //     eapp_print("OUT OF MEMORY: size %d %d\n", (int)size, free_cnt);
//     // }
//     // auto res = malloc(size);
//     // eapp_print("TOTAL: %d\n", (int)total);
//     return res;
// }

void* my_malloc(size_t size)
{
    // eapp_print("MALLOC: %d\n", (int)size);
#ifdef USE_TLSF
    if (g_mem) {
        void* ptr = tlsf_malloc(g_tlsf_pool, size);
        record(ptr, size);
        if (ptr == 0) {
            eapp_print("OOM! for %d\n", (int)size);
        }
        return ptr;
    }
    else
#endif
    {
        void* res = malloc(size);
        int check = 0;
        if (res == 0) eapp_print("OOM!\n");
        for (int i = 0; i < size; i++) check += ((char*)res)[i];
        if (check == 0) eapp_print("ALL ZERO\n");
        return res;
    }
    // return MemoryPoolAlloc(mp, size);
    // eapp_print("MALLOC: %d\n", size);
    //    return malloc(size);
    // return malloc(size);
    // static size_t total = 0;
    // total += size;
    // return malloc(size);

    // eapp_print("MALLOC %d\n", (int)size);
    // char* top = g_mem + g_pos;

    // if (size < 128) {
    //     return my_malloc_lite(size);
    // }
    // auto res = find_next_fit(size);
    // auto res = g_mem + g_pos;
    // g_pos += size;
    // // eapp_print("MALLOC: %d\n", (int)size);
    // if (g_pos >= HEAP_LEN) {
    //     // g_pos = 0;
    //     if (auto res = mem_pool.malloc(size)) {
    //         return res;
    //     }
    //     eapp_print("OUT OF MEMORY: size %d %d\n", (int)size, free_cnt);
    // }
    // // auto res = malloc(size);
    // // eapp_print("TOTAL: %d\n", (int)total);
    // return res;
}
std::unordered_map<void*, int> ptr_to_offset;
#define FREE(ptr)                    \
    do {                             \
        tlsf_free(g_tlsf_pool, ptr); \
        unrecord(ptr);               \
    } while (0)
void my_free(void* ptr)
{
    if (ptr_to_offset.find(ptr) != ptr_to_offset.end()) {
        free((char*)ptr - ptr_to_offset[ptr]);
        return;
    }
    // for (int i = 0; i < MAX_POXIS_MEMALIGN; ++i) {
    //     if (ptrs[i] != 0 && (char*)ptrs[i] + offset[i] == ptr) {
    //         FREE(ptrs[i]);
    //         eapp_print("FREE\n");
    //         ptrs[i] = 0;
    //         return;
    //     }
    // }
    // if (g_mem) tlsf_free(g_tlsf_pool, ptr);
#ifdef USE_TLSF
    if (g_mem && ptr >= g_mem && ptr < g_mem + HEAP_LEN) {
        // if (ptr >= mp->mlist->start && ptr < mp->mlist->start +
        // MEMPOOL_SIZE)
        // {
        FREE(ptr);
        // MemoryPoolFree(mp, ptr);
    }
    else
#endif
    {
        free(ptr);
    }
    // return;
    // if (mem_pool.contains(ptr)) {
    //     mem_pool.free(ptr);
    //     return;
    // }
    // if (ptr < g_mem || ptr >= g_mem + HEAP_LEN + 8) free(ptr);
    // free_block(ptr);
    // free(ptr);
    // free_cnt++;
}

void* operator new(unsigned long size)
{
    if (g_mem) return my_malloc(size);
    return malloc(size);
}

void operator delete(void* p) noexcept
{
    if (g_mem) {
        my_free(p);
    }
    else {
        free(p);
    }
}

void* operator new[](unsigned long size) { return operator new(size); }

void operator delete[](void* p) noexcept { operator delete(p); }

int my_posix_memalign(void** __memptr, size_t __alignment, size_t __size)
{
    eapp_print("POSIX MEMALIGN: %d %d\n", (int)__alignment, (int)__size);

#ifdef USE_TLSF
    if (g_mem) {
        *__memptr = tlsf_memalign(g_tlsf_pool, __alignment, __size);
        record(*__memptr, __size);
    }
    else
#endif
    {
        char* ptr = (char*)my_malloc(__size + __alignment - 1);
        char* next_align_ptr =
            (char*)(((size_t)ptr + __alignment - 1) & ~(__alignment - 1));
        ptr_to_offset[next_align_ptr] = next_align_ptr - ptr;
        *__memptr = next_align_ptr;
        int t = 0;
        for (char* p = (char*)next_align_ptr;
             p < (char*)next_align_ptr + __size; p++)
            t += *p;

        eapp_print("POSIX MEMALIGN: %d DONE\n", t);
        // posix_memalign(__memptr, __alignment, __size);
    }
    // eapp_print("SIZE: %d\n", (int)__size);
    // size_t size_needed = __size + __alignment - 1;
    // char* ptr = (char*)my_malloc(size_needed);
    // char* next_align_ptr =
    //     (char*)(((size_t)ptr + __alignment - 1) & ~(__alignment - 1));
    //
    // bool found = false;
    // for (int i = 0; i < MAX_POXIS_MEMALIGN; ++i) {
    //     if (ptrs[i] == 0) {
    //         ptrs[i] = ptr;
    //         offset[i] = next_align_ptr - ptr;
    //         found = true;
    //         break;
    //     }
    // }
    // if (!found) {
    //     eapp_print("NO SLOTS");
    // }
    // *__memptr = next_align_ptr;
    // if (__size > 100 * 1024 * 1024)
    // return posix_memalign(__memptr, __alignment, __size);
    // // return posix_memalign(__memptr, __alignment, __size);
    // char* next_align_ptr =
    //     (char*)(((size_t)g_mem + g_pos + __alignment - 1) & ~(__alignment -
    //     1));
    // g_pos = next_align_ptr - g_mem;
    // char* ptr = (char*)my_malloc(__size);
    // *__memptr = ptr;
    return 0;
}

void* my_calloc(size_t n, size_t m)
{
    void* ptr;
    my_posix_memalign(&ptr, 16, n * m);

    memset(ptr, 0, n * m);
    return ptr;
}

#ifdef __TEE
extern void* (*g_malloc)(size_t);
extern void (*g_free)(void*);

extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);
void manual_init_array(void)
{
    eapp_print("INIT ARRAY BEGIN\n");
    for (void (**p)() = __init_array_start; p < __init_array_end; ++p) {
        eapp_print("CALL INIT ARRAY: %p\n", *p);
        (*p)();
    }
    eapp_print("INIT ARRAY DONE\n");
}
#endif

int llm_inference(char* user_input, int user_input_len)
{
    eapp_print("HEAP: %p\n", g_mem);
    // g_mem = (char*)malloc(HEAP_LEN + 8);
    // ta_init(g_mem, g_mem + HEAP_LEN, 1024 * 512, 1024 * 64, 8);
    // g_tlsf_pool = tlsf_create_with_pool(g_mem, HEAP_LEN);
    // mp = MemoryPoolInit(MEMPOOL_SIZE, MEMPOOL_SIZE);
    // if (!mp) eapp_print("FAIL TO INIT MEM POOL\n");
    // my_malloc(100);
    // eapp_print("HEAP: %p\n", g_mem);
    // *(uint32_t*)g_mem = HEAP_LEN;
    // *(uint32_t*)((g_mem) + 4 + HEAP_LEN) = 0;
    // #ifdef __TEE
    //     g_malloc = my_malloc;
    //     g_free = my_free;
    // #endif
    //
    malloc(31);

    eapp_print("HEAP: %p\n", g_mem);
    // manual_init_array();
    g_io_helper.fread = my_fread;
    g_io_helper.fopen = my_fopen;
    g_io_helper.fclose = my_fclose;
    g_io_helper.fwrite = my_fwrite;
    g_io_helper.fseek = my_fseek;
    g_io_helper.ftell = my_ftell;
    // g_my_posix_memalign = my_posix_memalign;
    // g_my_malloc = my_malloc;
    // g_my_free = my_free;
    // g_my_calloc = my_calloc;
    g_my_posix_memalign = my_posix_memalign;
    g_my_malloc = my_malloc;
    g_my_free = my_free;
    g_my_calloc = my_calloc;

    run(user_input);

    eapp_print("MAX CNT: %d\n", max_cnt);
    return 0;
}
