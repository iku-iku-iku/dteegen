#include "embedding.h"

#include "../insecure/file.h"
#include "datareader.h"
// #include "mobilefacenet.bin.inc"
#include "mobilefacenet.id.h"
#include "mobilefacenet.mem.h"
// #include "mobilefacenet.param.inc"
#include <TEE-Capability/common.h>

#include "net.h"
#include "simpleocv.h"
// int __fprintf_chk(FILE *, int, const char *, ...);
// int __sprintf_chk(char *, int, size_t, const char *, ...);
#include <cstddef>
#include <cstdint>
#include <cstring>  // 用于memset
#include <filesystem>
#include <fstream>
#include <regex>
#include <unordered_map>
#include <vector>

#define INF 1000000

constexpr float THRESHOLD = 8;


class BitmapMemoryPool : public ncnn::Allocator
{
   private:
    void *pool;  // 内存池的起始地址
    size_t blockSize;
    size_t poolSize;
    size_t numBlocks;
    std::vector<uint8_t> bitmap;  // 位图，用于追踪每个块的占用情况
    std::unordered_map<size_t, size_t> allocatedMap;

   public:
    BitmapMemoryPool(void *memPool, size_t poolSize, size_t blockSize)
        : pool(memPool), poolSize(poolSize), blockSize(blockSize)
    {
        numBlocks = poolSize / blockSize;
        bitmap.resize((numBlocks + 7) / 8,
                      0);  // 初始化位图，所有位都设置为0（空闲）
    }

    virtual ~BitmapMemoryPool() override {}

    virtual void *fastMalloc(size_t size) override
    {
        size_t numBlocksNeeded =
            (size + blockSize - 1) / blockSize;  // 计算所需的块数
        size_t startBlock = findFreeBlocks(numBlocksNeeded);  // 查找连续空闲块

        if (startBlock == size_t(-1)) {
            eapp_print("NO BLOCKS\n");
            *(char *)0 = 0;
            return nullptr;  // 找不到足够的连续空闲块
        }

        // 标记这些块为已分配
        for (size_t i = 0; i < numBlocksNeeded; ++i) {
            setBlockUsed(startBlock + i);
        }

        allocatedMap.emplace(startBlock, numBlocksNeeded);
        // eapp_print("ALLOC: %d blocks at %d\n", numBlocksNeeded, startBlock);
        return static_cast<char *>(pool) + startBlock * blockSize;
    }

    virtual void fastFree(void *ptr) override
    {
        if (!ptr) return;  // 空指针，无需操作

        size_t blockIndex =
            (static_cast<char *>(ptr) - static_cast<char *>(pool)) / blockSize;
        size_t numBlocksNeeded = allocatedMap[blockIndex];
        for (size_t i = 0; i < numBlocksNeeded; ++i)
            setBlockFree(i + blockIndex);
    }

   private:
    // 查找连续的空闲块
    size_t findFreeBlocks(size_t numBlocksNeeded)
    {
        for (size_t i = 0; i <= numBlocks - numBlocksNeeded; ++i) {
            bool found = true;
            for (size_t j = 0; j < numBlocksNeeded; ++j) {
                if (isBlockUsed(i + j)) {
                    found = false;
                    i += j;  // 优化搜索，跳过检查过的已占用的块
                    break;
                }
            }
            if (found) {
                return i;
            }
        }
        return size_t(-1);  // 没有找到
    }

    bool isBlockUsed(size_t blockIndex)
    {
        return bitmap[blockIndex / 8] & (1 << (blockIndex % 8));
    }

    void setBlockUsed(size_t blockIndex)
    {
        bitmap[blockIndex / 8] |= (1 << (blockIndex % 8));
    }

    void setBlockFree(size_t blockIndex)
    {
        bitmap[blockIndex / 8] &= ~(1 << (blockIndex % 8));
    }
};

bool check_nan(float f)
{
    unsigned int *bits = (unsigned int *)&f;

    return (((*bits) & 0x7F800000) == 0x7F800000) &&
           (((*bits) & 0x007FFFFF) != 0);
}
void print_num(int num) { eapp_print("%d", num); }

void print_float(float num)
{
    // check num is nan
    if (check_nan(num)) {
        eapp_print("nan\n");
        return;
    }
    print_num((int)(num * 10000));
}
int embedding(in_char img[IMG_SIZE], out_char res[EMBEDDING_SIZE])
{
    ncnn::Net net;

#ifdef __TEE
    const int POOL_SIZE = 1024 * 1024 * 50;
    auto pool = new BitmapMemoryPool(malloc(POOL_SIZE), POOL_SIZE, 1024 * 16);
    // eapp_print("POOL CREATED\n");
    net.opt.use_vulkan_compute = false;
    net.opt.blob_allocator = pool;
    net.opt.workspace_allocator = pool;
#endif
    /* const unsigned char *mobilefacenet_param_ptr = mobilefacenet_param; */
    /* const unsigned char *mobilefacenet_bin_ptr = mobilefacenet_bin; */
    net.load_param(mobilefacenet_param_bin);
    eapp_print("LOADED PARAM\n");
    net.load_model(mobilefacenet_bin);
    eapp_print("LOADED MODEL\n");
    /* if
     * (net.load_param_bin(ncnn::DataReaderFromMemory(mobilefacenet_param_ptr)))
     */
    /*   exit(-1); */
    /* if (net.load_model(ncnn::DataReaderFromMemory(mobilefacenet_bin_ptr))) */
    /*   exit(-1); */

    ncnn::Mat input = ncnn::Mat::from_pixels(
        (const unsigned char *)img, ncnn::Mat::PIXEL_RGB, WIDTH, HEIGHT);
    for (int q = 0; q < input.c; q++) {
        float *ptr = input.channel(q);
        for (int i = 0; i < input.w * input.h; i++) {
            ptr[i] = ptr[i] * (1.f);
        }
    }

    ncnn::Mat output;
retry: {
    ncnn::Extractor extractor = net.create_extractor();
    extractor.input(mobilefacenet_param_id::BLOB_data, input);

    eapp_print("BEGIN INVOKE\n");
    extractor.extract(mobilefacenet_param_id::BLOB_fc1, output);
}

    ncnn::Mat out_flatterned = output.reshape(output.w * output.h * output.c);

    float *out = (float *)res;
    if (check_nan(out_flatterned[0])) goto retry;
    TEE_ASSERT(
        out_flatterned.w * out_flatterned.h * out_flatterned.c == EMB_LEN,
        "EMB_LEN: %d, OUT_FLATTERNED: %d\n", EMB_LEN,
        out_flatterned.w * out_flatterned.h * out_flatterned.c);
    for (int i = 0; i < EMB_LEN; i++) {
        out[i] = out_flatterned[i];
        if (i < 10) {
            eapp_print("THE EMB[%d] is %d", i, (int)(out[i] * 1000));
        }
    }
    eapp_print("DONE\n");

    /* return EMB_LEN * sizeof(float); */
    return seal_data_inplace((char *)res, EMBEDDING_SIZE,
                             EMB_LEN * sizeof(float));
}

float calculate_distance(in_char emb1[EMBEDDING_SIZE],
                         in_char emb2[EMBEDDING_SIZE])
{
    int emb_len1 = unseal_data_inplace(emb1, EMBEDDING_SIZE);
    int emb_len2 = unseal_data_inplace(emb2, EMBEDDING_SIZE);

    TEE_ASSERT(emb_len1 == emb_len2, "EMB1 LEN: %d, EMB2 LEN: %d\n", emb_len1,
               emb_len2);

    // emb_len1 /= sizeof(float);

    float *f1 = (float *)emb1;
    float *f2 = (float *)emb2;

    float sum = 0.f;
    for (int i = 0; i < EMB_LEN; i++) {
        float d = f1[i] - f2[i];
        sum += d * d;
    }

    return sum;
}

int img_recorder(in_char arr[IMG_SIZE], int id)
{
    char emb[EMBEDDING_SIZE];
    int sealed_data_len = embedding(arr, emb);
    // the embedding is sealed and can be stored safely.
    eapp_print("SEALED_LEN: %d, BUF_LEN: %d\n", sealed_data_len, EMBEDDING_SIZE);

    std::string filename = "emb" + std::to_string(id) + ".bin";
    eapp_print("FILENAME: %s\n", filename.c_str());

    write_file((char *)filename.c_str(), (int)filename.size() + 1, emb,
               sealed_data_len);
    // std::ofstream out(filename, std::ios::binary);
    // out.write(emb, sealed_data_len);
    return sealed_data_len;
}

int img_verifier(in_char arr[IMG_SIZE])
{
    char recorded_face_emb[EMBEDDING_SIZE];
    char in_face_emb[EMBEDDING_SIZE];

    int sealed_data_len = embedding(arr, in_face_emb);
    float min_dist = INF;
    int min_dist_id = -1;

    int emb_ids[MAX_EMB_CNT];
    int emb_cnt = get_emb_list((char *)emb_ids);
    for (int i = 0; i < emb_cnt; i++) {
        int emb_id = emb_ids[i];
        std::string filename = "emb" + std::to_string(emb_id) + ".bin";
        read_file((char *)filename.c_str(), (int)filename.size(),
                  recorded_face_emb, EMBEDDING_SIZE);
        float dist = calculate_distance(recorded_face_emb, in_face_emb);
        eapp_print("DISTANCE WITH PERSON%d: %d x 10^-3\n", emb_id,
                   (int)(1000 * dist));

        if (dist < min_dist) {
            min_dist = dist;
            min_dist_id = emb_id;
        }
    }
    // for (const auto &e : std::filesystem::directory_iterator(".")) {
    //     const auto &path = e.path();
    //     const auto path_str = path.string();
    //     // if path_str belike "emb*.bin", take its id and do a comparison in
    //     // enclave
    //     std::regex re("emb(\\d+)\\.bin");
    //     if (std::regex_search(path_str, re)) {
    //         std::smatch match;
    //         std::regex_search(path_str, match, re);
    //         int id = std::stoi(match[1]);
    //         std::ifstream in(path_str, std::ios::binary);
    //         in.read(emb1, EMBEDDING_SIZE);

    //         memcpy(emb2, in_emb, EMBEDDING_SIZE);
    //         float dist;
    //         // calculate_distance(emb1, emb2, (char *)&dist);
    //         dist = calculate_distance(emb1, emb2);
    //         printf("DISTANCE WITH PERSON%d: %f\n", id, dist);

    //         if (dist < min_dist) {
    //             min_dist = dist;
    //             min_dist_id = id;
    //         }
    //     }
    // }

    if (min_dist < THRESHOLD * THRESHOLD) {
        return min_dist_id;
    }
    return -1;
}