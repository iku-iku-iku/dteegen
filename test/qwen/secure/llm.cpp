#include "llm.h"

#include <TEE-Capability/common.h>

#include <fstream>
#include <sstream>
#include <vector>

#include "../insecure/file_stub.h"
#include "common/common.h"
#include "llama.h"
/* int open_file(char *filename, int filename_len, char buf[1024]) { */
/*   std::ifstream ifs(filename); */
/* } */

#define BUF_LEN 1024

int run(const char* prompt)
{
    gpt_params params;
    params.n_threads = 1;
    /* params.prompt = "<user>who are you<model>\n"; */
    params.prompt = prompt;
    params.model = "qwen-model";

    params.seed = 691;
    llama_sampling_params& sparams = params.sparams;

    if (params.n_ctx != 0 && params.n_ctx < 8) {
        LOG_TEE("%s: warning: minimum context size is 8, using minimum size.\n",
                __func__);
        params.n_ctx = 8;
    }

    if (params.rope_freq_base != 0.0) {
        LOG_TEE("%s: warning: changing RoPE frequency base to %g.\n", __func__,
                params.rope_freq_base);
    }

    if (params.rope_freq_scale != 0.0) {
        LOG_TEE("%s: warning: scaling RoPE frequency by %g.\n", __func__,
                params.rope_freq_scale);
    }

    LOG_TEE("%s: build = %d (%s)\n", __func__, LLAMA_BUILD_NUMBER,
            LLAMA_COMMIT);
    LOG_TEE("%s: built with %s for %s\n", __func__, LLAMA_COMPILER,
            LLAMA_BUILD_TARGET);

    if (params.seed == LLAMA_DEFAULT_SEED) {
        params.seed = time(NULL);
    }

    LOG_TEE("%s: seed  = %u\n", __func__, params.seed);

    std::mt19937 rng(params.seed);
    if (params.random_prompt) {
        params.prompt = gpt_random_prompt(rng);
    }

    LOG("%s: llama backend init\n", __func__);
    llama_backend_init();
    llama_numa_init(params.numa);

    llama_model* model;
    llama_context* ctx;
    llama_context* ctx_guidance = NULL;
    auto g_model = &model;
    auto g_ctx = &ctx;

    // load the model and apply lora adapter, if any
    LOG("%s: load the model and apply lora adapter, if any\n", __func__);
    std::tie(model, ctx) = llama_init_from_gpt_params(params);
    if (sparams.cfg_scale > 1.f) {
        struct llama_context_params lparams =
            llama_context_params_from_gpt_params(params);
        ctx_guidance = llama_new_context_with_model(model, lparams);
    }

    if (model == NULL) {
        LOG_TEE("%s: error: unable to load model\n", __func__);
        return 1;
    }

    const int n_ctx_train = llama_n_ctx_train(model);
    const int n_ctx = llama_n_ctx(ctx);
    LOG("n_ctx: %d\n", n_ctx);

    if (n_ctx > n_ctx_train) {
        LOG_TEE(
            "%s: warning: model was trained on only %d context tokens (%d "
            "specified)\n",
            __func__, n_ctx_train, n_ctx);
    }

    // print system information
    {
        LOG_TEE("\n");
        LOG_TEE("%s\n", get_system_info(params).c_str());
    }

    std::string path_session = params.path_prompt_cache;
    std::vector<llama_token> session_tokens;

    if (!path_session.empty()) {
        LOG_TEE("%s: attempting to load saved session from '%s'\n", __func__,
                path_session.c_str());
        {
            // The file exists and is not empty
            session_tokens.resize(n_ctx);
            size_t n_token_count_out = 0;
            if (!llama_load_session_file(
                    ctx, path_session.c_str(), session_tokens.data(),
                    session_tokens.capacity(), &n_token_count_out)) {
                LOG_TEE("%s: error: failed to load session file '%s'\n",
                        __func__, path_session.c_str());
                return 1;
            }
            session_tokens.resize(n_token_count_out);
            llama_set_rng_seed(ctx, params.seed);
            LOG_TEE("%s: loaded a session with prompt size of %d tokens\n",
                    __func__, (int)session_tokens.size());
        }
    }

    const bool add_bos = llama_should_add_bos_token(model);
    LOG("add_bos: %d\n", add_bos);

    std::vector<llama_token> embd_inp;

    if (params.interactive_first || params.instruct || params.chatml ||
        !params.prompt.empty() || session_tokens.empty()) {
        LOG("tokenize the prompt\n");
        if (params.chatml) {
            params.prompt =
                "<|im_start|>system\n" + params.prompt + "<|im_end|>";
        }
        embd_inp = ::llama_tokenize(ctx, params.prompt, add_bos, true);
    }
    else {
        LOG("use session tokens\n");
        embd_inp = session_tokens;
    }

    LOG("prompt: \"%s\"\n", log_tostr(params.prompt));
    LOG("tokens: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, embd_inp).c_str());

    // Should not run without any tokens
    if (embd_inp.empty()) {
        embd_inp.push_back(llama_token_bos(model));
        LOG("embd_inp was considered empty and bos was added: %s\n",
            LOG_TOKENS_TOSTR_PRETTY(ctx, embd_inp).c_str());
    }

    // Tokenize negative prompt
    std::vector<llama_token> guidance_inp;
    int guidance_offset = 0;
    int original_prompt_len = 0;
    if (ctx_guidance) {
        LOG("cfg_negative_prompt: \"%s\"\n",
            log_tostr(sparams.cfg_negative_prompt));

        guidance_inp = ::llama_tokenize(
            ctx_guidance, sparams.cfg_negative_prompt, add_bos, true);
        LOG("guidance_inp tokenized: %s\n",
            LOG_TOKENS_TOSTR_PRETTY(ctx_guidance, guidance_inp).c_str());

        std::vector<llama_token> original_inp =
            ::llama_tokenize(ctx, params.prompt, add_bos, true);
        LOG("original_inp tokenized: %s\n",
            LOG_TOKENS_TOSTR_PRETTY(ctx, original_inp).c_str());

        original_prompt_len = original_inp.size();
        guidance_offset = (int)guidance_inp.size() - original_prompt_len;
        LOG("original_prompt_len: %s", log_tostr(original_prompt_len));
        LOG("guidance_offset:     %s", log_tostr(guidance_offset));
    }

    if ((int)embd_inp.size() > n_ctx - 4) {
        LOG_TEE("%s: error: prompt is too long (%d tokens, max %d)\n", __func__,
                (int)embd_inp.size(), n_ctx - 4);
        return 1;
    }

    // debug message about similarity of saved session, if applicable
    size_t n_matching_session_tokens = 0;
    if (!session_tokens.empty()) {
        for (llama_token id : session_tokens) {
            if (n_matching_session_tokens >= embd_inp.size() ||
                id != embd_inp[n_matching_session_tokens]) {
                break;
            }
            n_matching_session_tokens++;
        }
        if (params.prompt.empty() &&
            n_matching_session_tokens == embd_inp.size()) {
            LOG_TEE("%s: using full prompt from session file\n", __func__);
        }
        else if (n_matching_session_tokens >= embd_inp.size()) {
            LOG_TEE("%s: session file has exact match for prompt!\n", __func__);
        }
        else if (n_matching_session_tokens < (embd_inp.size() / 2)) {
            LOG_TEE(
                "%s: warning: session file has low similarity to prompt (%zu / "
                "%zu tokens); will mostly be reevaluated\n",
                __func__, n_matching_session_tokens, embd_inp.size());
        }
        else {
            LOG_TEE("%s: session file matches %zu / %zu tokens of prompt\n",
                    __func__, n_matching_session_tokens, embd_inp.size());
        }

        // remove any "future" tokens that we might have inherited from the
        // previous session
        llama_kv_cache_seq_rm(ctx, -1, n_matching_session_tokens, -1);
    }

    LOGLN(
        "recalculate the cached logits (check): embd_inp.empty() %s, "
        "n_matching_session_tokens %zu, embd_inp.size() %zu, "
        "session_tokens.size() %zu, embd_inp.size() %zu",
        log_tostr(embd_inp.empty()), n_matching_session_tokens, embd_inp.size(),
        session_tokens.size(), embd_inp.size());

    // if we will use the cache for the full prompt without reaching the end of
    // the cache, force reevaluation of the last token token to recalculate the
    // cached logits
    if (!embd_inp.empty() && n_matching_session_tokens == embd_inp.size() &&
        session_tokens.size() > embd_inp.size()) {
        LOGLN(
            "recalculate the cached logits (do): session_tokens.resize( %zu )",
            embd_inp.size() - 1);

        session_tokens.resize(embd_inp.size() - 1);
    }

    // number of tokens to keep when resetting context
    if (params.n_keep < 0 || params.n_keep > (int)embd_inp.size() ||
        params.instruct || params.chatml) {
        params.n_keep = (int)embd_inp.size();
    }
    else {
        params.n_keep += add_bos;  // always keep the BOS token
    }

    // prefix & suffix for instruct mode
    const auto inp_pfx =
        ::llama_tokenize(ctx, "\n\n### Instruction:\n\n", add_bos, true);
    const auto inp_sfx =
        ::llama_tokenize(ctx, "\n\n### Response:\n\n", false, true);

    LOG("inp_pfx: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, inp_pfx).c_str());
    LOG("inp_sfx: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, inp_sfx).c_str());

    // chatml prefix & suffix
    const auto cml_pfx =
        ::llama_tokenize(ctx, "\n<|im_start|>user\n", add_bos, true);
    const auto cml_sfx = ::llama_tokenize(
        ctx, "<|im_end|>\n<|im_start|>assistant\n", false, true);

    LOG("cml_pfx: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, cml_pfx).c_str());
    LOG("cml_sfx: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, cml_sfx).c_str());

    // in instruct mode, we inject a prefix and a suffix to each input by the
    // user
    if (params.instruct) {
        params.interactive_first = true;
        params.antiprompt.emplace_back("### Instruction:\n\n");
    }
    // similar for chatml mode
    else if (params.chatml) {
        params.interactive_first = true;
        params.antiprompt.emplace_back("<|im_start|>user\n");
    }

    // enable interactive mode if interactive start is specified
    /* if (params.interactive_first) { */
    /*     params.interactive = true; */
    /* } */

    /* if (params.verbose_prompt) { */
    /*     LOG_TEE("\n"); */
    /*     LOG_TEE("%s: prompt: '%s'\n", __func__, params.prompt.c_str()); */
    /*     LOG_TEE("%s: number of tokens in prompt = %zu\n", __func__, */
    /*             embd_inp.size()); */
    /*     for (int i = 0; i < (int)embd_inp.size(); i++) { */
    /*         LOG_TEE("%6d -> '%s'\n", embd_inp[i], */
    /*                 llama_token_to_piece(ctx, embd_inp[i]).c_str()); */
    /*     } */
    /**/
    /*     if (ctx_guidance) { */
    /*         LOG_TEE("\n"); */
    /*         LOG_TEE("%s: negative prompt: '%s'\n", __func__, */
    /*                 sparams.cfg_negative_prompt.c_str()); */
    /*         LOG_TEE("%s: number of tokens in negative prompt = %zu\n",
     * __func__, */
    /*                 guidance_inp.size()); */
    /*         for (int i = 0; i < (int)guidance_inp.size(); i++) { */
    /*             LOG_TEE("%6d -> '%s'\n", guidance_inp[i], */
    /*                     llama_token_to_piece(ctx, guidance_inp[i]).c_str());
     */
    /*         } */
    /*     } */
    /**/
    /*     if (params.n_keep > add_bos) { */
    /*         LOG_TEE("%s: static prompt based on n_keep: '", __func__); */
    /*         for (int i = 0; i < params.n_keep; i++) { */
    /*             LOG_TEE("%s", llama_token_to_piece(ctx,
     * embd_inp[i]).c_str()); */
    /*         } */
    /*         LOG_TEE("'\n"); */
    /*     } */
    /*     LOG_TEE("\n"); */
    /* } */

    /* if (params.interactive) { */
    /*     LOG_TEE("%s: interactive mode on.\n", __func__); */
    /**/
    /*     if (!params.antiprompt.empty()) { */
    /*         for (const auto& antiprompt : params.antiprompt) { */
    /*             LOG_TEE("Reverse prompt: '%s'\n", antiprompt.c_str()); */
    /*             if (params.verbose_prompt) { */
    /*                 auto tmp = ::llama_tokenize(ctx, antiprompt, false,
     * true); */
    /*                 for (int i = 0; i < (int)tmp.size(); i++) { */
    /*                     LOG_TEE("%6d -> '%s'\n", tmp[i], */
    /*                             llama_token_to_piece(ctx, tmp[i]).c_str());
     */
    /*                 } */
    /*             } */
    /*         } */
    /*     } */
    /**/
    /*     if (params.input_prefix_bos) { */
    /*         LOG_TEE("Input prefix with BOS\n"); */
    /*     } */
    /**/
    /*     if (!params.input_prefix.empty()) { */
    /*         LOG_TEE("Input prefix: '%s'\n", params.input_prefix.c_str()); */
    /*         if (params.verbose_prompt) { */
    /*             auto tmp = */
    /*                 ::llama_tokenize(ctx, params.input_prefix, true, true);
     */
    /*             for (int i = 0; i < (int)tmp.size(); i++) { */
    /*                 LOG_TEE("%6d -> '%s'\n", tmp[i], */
    /*                         llama_token_to_piece(ctx, tmp[i]).c_str()); */
    /*             } */
    /*         } */
    /*     } */
    /**/
    /*     if (!params.input_suffix.empty()) { */
    /*         LOG_TEE("Input suffix: '%s'\n", params.input_suffix.c_str()); */
    /*         if (params.verbose_prompt) { */
    /*             auto tmp = */
    /*                 ::llama_tokenize(ctx, params.input_suffix, false, true);
     */
    /*             for (int i = 0; i < (int)tmp.size(); i++) { */
    /*                 LOG_TEE("%6d -> '%s'\n", tmp[i], */
    /*                         llama_token_to_piece(ctx, tmp[i]).c_str()); */
    /*             } */
    /*         } */
    /*     } */
    /* } */
    /* LOG_TEE("sampling: \n%s\n", llama_sampling_print(sparams).c_str()); */
    /* LOG_TEE("sampling order: \n%s\n", */
    /*         llama_sampling_order_print(sparams).c_str()); */
    /* LOG_TEE("generate: n_ctx = %d, n_batch = %d, n_predict = %d, n_keep =
     * %d\n", */
    /*         n_ctx, params.n_batch, params.n_predict, params.n_keep); */

    // group-attention state
    // number of grouped KV tokens so far (used only if params.grp_attn_n > 1)
    int ga_i = 0;

    const int ga_n = params.grp_attn_n;
    const int ga_w = params.grp_attn_w;

    if (ga_n != 1) {
        GGML_ASSERT(ga_n > 0 && "grp_attn_n must be positive");  // NOLINT
        GGML_ASSERT(ga_w % ga_n == 0 &&
                    "grp_attn_w must be a multiple of grp_attn_n");  // NOLINT
        // GGML_ASSERT(n_ctx_train % ga_w == 0     && "n_ctx_train must be a
        // multiple of grp_attn_w");    // NOLINT GGML_ASSERT(n_ctx >=
        // n_ctx_train * ga_n && "n_ctx must be at least n_ctx_train *
        // grp_attn_n"); // NOLINT
        LOG_TEE(
            "self-extend: n_ctx_train = %d, grp_attn_n = %d, grp_attn_w = %d\n",
            n_ctx_train, ga_n, ga_w);
    }
    LOG_TEE("\n\n");

    if (params.interactive) {
        const char* control_message;
        if (params.multiline_input) {
            control_message =
                " - To return control to LLaMa, end your input with '\\'.\n"
                " - To return control without starting a new line, end your "
                "input with '/'.\n";
        }
        else {
            control_message =
                " - Press Return to return control to LLaMa.\n"
                " - To return control without starting a new line, end your "
                "input with '/'.\n"
                " - If you want to submit another line, end your input with "
                "'\\'.\n";
        }
        LOG_TEE("== Running in interactive mode. ==\n");
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)) || \
    defined(_WIN32)
        LOG_TEE(" - Press Ctrl+C to interject at any time.\n");
#endif
        LOG_TEE("%s\n", control_message);
    }

    bool is_antiprompt = false;
    bool input_echo = true;
    bool display = true;
    bool need_to_save_session =
        !path_session.empty() && n_matching_session_tokens < embd_inp.size();

    int n_past = 0;
    int n_remain = params.n_predict;
    int n_consumed = 0;
    int n_session_consumed = 0;
    int n_past_guidance = 0;

    std::vector<int> input_tokens;
    auto g_input_tokens = &input_tokens;
    std::vector<int> output_tokens;
    auto g_output_tokens = &output_tokens;
    std::ostringstream output_ss;
    auto g_output_ss = &output_ss;

    // the first thing we will do is to output the prompt, so set color
    // accordingly
    display = params.display_prompt;

    std::vector<llama_token> embd;
    std::vector<llama_token> embd_guidance;

    struct llama_sampling_context* ctx_sampling = llama_sampling_init(sparams);

    while ((n_remain != 0 && !is_antiprompt) || params.interactive) {
        // predict
        if (!embd.empty()) {
            // Note: (n_ctx - 4) here is to match the logic for commandline
            // prompt handling via
            // --prompt or --file which uses the same value.
            int max_embd_size = n_ctx - 4;

            // Ensure the input doesn't exceed the context size by truncating
            // embd if necessary.
            if ((int)embd.size() > max_embd_size) {
                const int skipped_tokens = (int)embd.size() - max_embd_size;
                embd.resize(max_embd_size);

                fflush(stdout);
            }

            if (ga_n == 1) {
                // infinite text generation via context shifting
                // if we run out of context:
                // - take the n_keep first tokens from the original prompt (via
                // n_past)
                // - take half of the last (n_ctx - n_keep) tokens and recompute
                // the logits in batches
                if (n_past + (int)embd.size() +
                        std::max<int>(0, guidance_offset) >
                    n_ctx) {
                    if (params.n_predict == -2) {
                        LOG_TEE(
                            "\n\n%s: context full and n_predict == -%d => "
                            "stopping\n",
                            __func__, params.n_predict);
                        break;
                    }

                    const int n_left = n_past - params.n_keep;
                    const int n_discard = n_left / 2;

                    LOG("context full, swapping: n_past = %d, n_left = %d, "
                        "n_ctx = %d, n_keep = %d, n_discard = %d\n",
                        n_past, n_left, n_ctx, params.n_keep, n_discard);

                    llama_kv_cache_seq_rm(ctx, 0, params.n_keep,
                                          params.n_keep + n_discard);
                    llama_kv_cache_seq_add(ctx, 0, params.n_keep + n_discard,
                                           n_past, -n_discard);

                    n_past -= n_discard;

                    if (ctx_guidance) {
                        n_past_guidance -= n_discard;
                    }

                    LOG("after swap: n_past = %d, n_past_guidance = %d\n",
                        n_past, n_past_guidance);

                    LOG("embd: %s\n",
                        LOG_TOKENS_TOSTR_PRETTY(ctx, embd).c_str());

                    LOG("clear session path\n");
                    path_session.clear();
                }
            }
            else {
                // context extension via Self-Extend
                while (n_past >= ga_i + ga_w) {
                    const int ib = (ga_n * ga_i) / ga_w;
                    const int bd = (ga_w / ga_n) * (ga_n - 1);
                    const int dd = (ga_w / ga_n) - ib * bd - ga_w;

                    LOG("\n");
                    LOG("shift: [%6d, %6d] + %6d -> [%6d, %6d]\n", ga_i, n_past,
                        ib * bd, ga_i + ib * bd, n_past + ib * bd);
                    LOG("div:   [%6d, %6d] / %6d -> [%6d, %6d]\n",
                        ga_i + ib * bd, ga_i + ib * bd + ga_w, ga_n,
                        (ga_i + ib * bd) / ga_n,
                        (ga_i + ib * bd + ga_w) / ga_n);
                    LOG("shift: [%6d, %6d] + %6d -> [%6d, %6d]\n",
                        ga_i + ib * bd + ga_w, n_past + ib * bd, dd,
                        ga_i + ib * bd + ga_w + dd, n_past + ib * bd + dd);

                    llama_kv_cache_seq_add(ctx, 0, ga_i, n_past, ib * bd);
                    llama_kv_cache_seq_div(ctx, 0, ga_i + ib * bd,
                                           ga_i + ib * bd + ga_w, ga_n);
                    llama_kv_cache_seq_add(ctx, 0, ga_i + ib * bd + ga_w,
                                           n_past + ib * bd, dd);

                    n_past -= bd;

                    ga_i += ga_w / ga_n;

                    LOG("\nn_past_old = %d, n_past = %d, ga_i = %d\n\n",
                        n_past + bd, n_past, ga_i);
                }
            }

            // try to reuse a matching prefix from the loaded session instead of
            // re-eval (via n_past)
            if (n_session_consumed < (int)session_tokens.size()) {
                size_t i = 0;
                for (; i < embd.size(); i++) {
                    if (embd[i] != session_tokens[n_session_consumed]) {
                        session_tokens.resize(n_session_consumed);
                        break;
                    }

                    n_past++;
                    n_session_consumed++;

                    if (n_session_consumed >= (int)session_tokens.size()) {
                        ++i;
                        break;
                    }
                }
                if (i > 0) {
                    embd.erase(embd.begin(), embd.begin() + i);
                }
            }

            // evaluate tokens in batches
            // embd is typically prepared beforehand to fit within a batch, but
            // not always
            if (ctx_guidance) {
                int input_size = 0;
                llama_token* input_buf = NULL;

                if (n_past_guidance < (int)guidance_inp.size()) {
                    // Guidance context should have the same data with these
                    // modifications:
                    //
                    // * Replace the initial prompt
                    // * Shift everything by guidance_offset
                    embd_guidance = guidance_inp;
                    if (embd.begin() + original_prompt_len < embd.end()) {
                        embd_guidance.insert(embd_guidance.end(),
                                             embd.begin() + original_prompt_len,
                                             embd.end());
                    }

                    input_buf = embd_guidance.data();
                    input_size = embd_guidance.size();

                    LOG("guidance context: %s\n",
                        LOG_TOKENS_TOSTR_PRETTY(ctx, embd_guidance).c_str());
                }
                else {
                    input_buf = embd.data();
                    input_size = embd.size();
                }

                for (int i = 0; i < input_size; i += params.n_batch) {
                    int n_eval = std::min(input_size - i, params.n_batch);
                    if (llama_decode(ctx_guidance,
                                     llama_batch_get_one(input_buf + i, n_eval,
                                                         n_past_guidance, 0))) {
                        LOG_TEE("%s : failed to eval\n", __func__);
                        return 1;
                    }

                    n_past_guidance += n_eval;
                }
            }

            for (int i = 0; i < (int)embd.size(); i += params.n_batch) {
                int n_eval = (int)embd.size() - i;
                if (n_eval > params.n_batch) {
                    n_eval = params.n_batch;
                }

                LOG("eval: %s\n", LOG_TOKENS_TOSTR_PRETTY(ctx, embd).c_str());

                if (llama_decode(ctx, llama_batch_get_one(&embd[i], n_eval,
                                                          n_past, 0))) {
                    LOG_TEE("%s : failed to eval\n", __func__);
                    return 1;
                }

                n_past += n_eval;

                LOG("n_past = %d\n", n_past);
                // Display total tokens alongside total time
                if (params.n_print > 0 && n_past % params.n_print == 0) {
                    LOG_TEE(
                        "\n\033[31mTokens consumed so far = %d / %d \033[0m\n",
                        n_past, n_ctx);
                }
            }

            if (!embd.empty() && !path_session.empty()) {
                session_tokens.insert(session_tokens.end(), embd.begin(),
                                      embd.end());
                n_session_consumed = session_tokens.size();
            }
        }

        embd.clear();
        embd_guidance.clear();

        if ((int)embd_inp.size() <= n_consumed) {
            // optionally save the session on first sample (for faster prompt
            // loading next time)
            if (!path_session.empty() && need_to_save_session &&
                !params.prompt_cache_ro) {
                need_to_save_session = false;
                llama_save_session_file(ctx, path_session.c_str(),
                                        session_tokens.data(),
                                        session_tokens.size());

                LOG("saved session to %s\n", path_session.c_str());
            }

            const llama_token id =
                llama_sampling_sample(ctx_sampling, ctx, ctx_guidance);

            llama_sampling_accept(ctx_sampling, ctx, id, true);

            LOG("last: %s\n",
                LOG_TOKENS_TOSTR_PRETTY(ctx, ctx_sampling->prev).c_str());

            embd.push_back(id);

            // echo this to console
            input_echo = true;

            // decrement remaining sampling budget
            --n_remain;

            LOG("n_remain: %d\n", n_remain);
        }
        else {
            // some user input remains from prompt or interaction, forward it to
            // processing
            LOG("embd_inp.size(): %d, n_consumed: %d\n", (int)embd_inp.size(),
                n_consumed);
            while ((int)embd_inp.size() > n_consumed) {
                embd.push_back(embd_inp[n_consumed]);

                // push the prompt in the sampling context in order to apply
                // repetition penalties later for the prompt, we don't apply
                // grammar rules
                llama_sampling_accept(ctx_sampling, ctx, embd_inp[n_consumed],
                                      false);

                ++n_consumed;
                if ((int)embd.size() >= params.n_batch) {
                    break;
                }
            }
        }

        // display text
        if (input_echo && display) {
            for (auto id : embd) {
                const std::string token_str = llama_token_to_piece(ctx, id);
                printf("%s", token_str.c_str());

                if (embd.size() > 1) {
                    input_tokens.push_back(id);
                }
                else {
                    output_tokens.push_back(id);
                    output_ss << token_str;
                }
            }
            fflush(stdout);
        }
        // reset color to default if there is no pending user input
        if (input_echo && (int)embd_inp.size() == n_consumed) {
            display = true;
        }

        // if not currently processing queued inputs;
        if ((int)embd_inp.size() <= n_consumed) {
            // check for reverse prompt in the last n_prev tokens
            if (!params.antiprompt.empty()) {
                const int n_prev = 32;
                const std::string last_output =
                    llama_sampling_prev_str(ctx_sampling, ctx, n_prev);

                is_antiprompt = false;
                // Check if each of the reverse prompts appears at the end of
                // the output. If we're not running interactively, the reverse
                // prompt might be tokenized with some following characters so
                // we'll compensate for that by widening the search window a
                // bit.
                for (std::string& antiprompt : params.antiprompt) {
                    size_t extra_padding = params.interactive ? 0 : 2;
                    size_t search_start_pos =
                        last_output.length() >
                                static_cast<size_t>(antiprompt.length() +
                                                    extra_padding)
                            ? last_output.length() -
                                  static_cast<size_t>(antiprompt.length() +
                                                      extra_padding)
                            : 0;

                    if (last_output.find(antiprompt, search_start_pos) !=
                        std::string::npos) {
                        if (params.interactive) {
                        }
                        is_antiprompt = true;
                        break;
                    }
                }

                if (is_antiprompt) {
                    LOG("found antiprompt: %s\n", last_output.c_str());
                }
            }
        }

        // end of text token
        if (!embd.empty() && embd.back() == llama_token_eos(model) &&
            !(params.instruct || params.interactive || params.chatml)) {
            LOG_TEE(" [end of text]\n");
            break;
        }

        // In interactive mode, respect the maximum number of tokens and drop
        // back to user input when reached. We skip this logic when n_predict ==
        // -1 (infinite) or -2 (stop at context size).
        if (params.interactive && n_remain <= 0 && params.n_predict >= 0) {
            n_remain = params.n_predict;
        }
    }

    /* if (!path_session.empty() && params.prompt_cache_all && */
    /*     !params.prompt_cache_ro) { */
    /*     LOG_TEE("\n%s: saving final output to session file '%s'\n", __func__,
     */
    /*             path_session.c_str()); */
    /*     llama_save_session_file(ctx, path_session.c_str(), */
    /*                             session_tokens.data(),
     * session_tokens.size()); */
    /* } */

    /* llama_print_timings(ctx); */

    if (ctx_guidance) {
        llama_free(ctx_guidance);
    }
    llama_free(ctx);
    llama_free_model(model);

    llama_sampling_free(ctx_sampling);
    llama_backend_free();

    return 0;
}

int llm_inference(char* user_input, int user_input_len)
{
    run(user_input);
    char filename[] = "model";

    char buf[BUF_LEN];
    int read_len = read_file(filename, sizeof(filename), 0, buf, sizeof(buf));
    for (int i = 0; i < read_len; i++) {
        eapp_print("buf[%d]: %c\n", i, buf[i]);
    }
    return 0;
}
