/**
 * Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang)
 * Copyright (c)  2022                     (Pingfeng Luo)
 *
 * See LICENSE for clarification regarding multiple authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <TEE-Capability/common.h>

#include <algorithm>
#include <chrono>  // NOLINT
#include <fstream>
#include <iostream>

#include "decoder_jit_trace-pnnx.ncnn.id.h"
#include "decoder_jit_trace-pnnx.ncnn.mem.h"
#include "encoder_jit_trace-pnnx.ncnn.id.h"
#include "encoder_jit_trace-pnnx.ncnn.mem.h"
#include "joiner_jit_trace-pnnx.ncnn.id.h"
#include "joiner_jit_trace-pnnx.ncnn.mem.h"
#include "net.h"  // NOLINT
#include "sherpa-ncnn/csrc/recognizer.h"
#include "sherpa-ncnn/csrc/wave-reader.h"
#include "tokens.h"
#include "wav.h"

int speech_recognition()
{
    sherpa_ncnn::RecognizerConfig config;
    config.model_config.encoder_param_data =
        encoder_jit_trace_pnnx_ncnn_param_bin;
    config.model_config.encoder_bin_data = encoder_jit_trace_pnnx_ncnn_bin;
    config.model_config.decoder_param_data =
        decoder_jit_trace_pnnx_ncnn_param_bin;
    config.model_config.decoder_bin_data = decoder_jit_trace_pnnx_ncnn_bin;
    config.model_config.joiner_param_data =
        joiner_jit_trace_pnnx_ncnn_param_bin;
    config.model_config.joiner_bin_data = joiner_jit_trace_pnnx_ncnn_bin;
    config.model_config.tokens_data = tokens_txt;
    config.model_config.tokens_size = tokens_txt_len;

    int32_t num_threads = 1;
    config.model_config.encoder_opt.num_threads = num_threads;
    config.model_config.decoder_opt.num_threads = num_threads;
    config.model_config.joiner_opt.num_threads = num_threads;

    float expected_sampling_rate = 16000;

    config.feat_config.sampling_rate = expected_sampling_rate;
    config.feat_config.feature_dim = 80;

    sherpa_ncnn::Recognizer recognizer(config);

    bool is_ok = false;
    std::vector<float> samples = sherpa_ncnn::ReadWave(
        __0_wav, __0_wav_len, expected_sampling_rate, &is_ok);
    if (!is_ok) {
        exit(-1);
    }

    const float duration = samples.size() / expected_sampling_rate;
    auto stream = recognizer.CreateStream();
    stream->AcceptWaveform(expected_sampling_rate, samples.data(),
                           samples.size());
    std::vector<float> tail_paddings(
        static_cast<int>(0.3 * expected_sampling_rate));
    stream->AcceptWaveform(expected_sampling_rate, tail_paddings.data(),
                           tail_paddings.size());

    while (recognizer.IsReady(stream.get())) {
        recognizer.DecodeStream(stream.get());
    }

    auto result = recognizer.GetResult(stream.get());

    eapp_print("%s\n", result.ToString().c_str());

    return 0;
}
