#include "embedding.h"
#include "MobileFaceNet.inc"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include <TEE-Capability/common.h>

const int S = 100000000;

int embedding(in_char img[IMG_SIZE], out_char res[EMBEDDING_SIZE]) {
  for (int i = 0; i < 5; i++) {
    eapp_print("img[%d] %d\n", i, (int)img[i]);
  }
  // Load model
  std::unique_ptr<tflite::FlatBufferModel> model =
      tflite::FlatBufferModel::BuildFromBuffer((const char *)MobileFaceNet,
                                               MobileFaceNet_len);
  TEE_CHECK(model != nullptr);
  tflite::ops::builtin::BuiltinOpResolver resolver;
  tflite::InterpreterBuilder builder(*model, resolver);
  std::unique_ptr<tflite::Interpreter> interpreter;
  builder(&interpreter);
  TEE_CHECK(interpreter != nullptr);

  // Allocate tensor buffers.
  eapp_print("BEGIN ALLOCATE TENSORS\n");
  TEE_CHECK(interpreter->AllocateTensors() == kTfLiteOk);

  TfLiteTensor *input_tensor = interpreter->tensor(interpreter->inputs()[0]);
  float *input_data = input_tensor->data.f;
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      for (int k = 0; k < 3; k++) {
        int idx = i * HEIGHT * 3 + j * 3 + k;
        input_data[idx] = img[idx] / 255.0f;
      }
    }
  }

  eapp_print("BEGIN INVOKE\n");
  TEE_CHECK(interpreter->Invoke() == kTfLiteOk);

  eapp_print("DONE\n");

  TfLiteTensor *output_tensor = interpreter->tensor(interpreter->outputs()[0]);
  float *out = (float *)res;
  for (int i = 0; i < EMB_LEN; i++) {
    out[i] = output_tensor->data.f[i];
  }
  return EMB_LEN * sizeof(float);
  // return seal_data_inplace((char *)res, EMBEDDING_SIZE,
  //                          EMB_LEN * sizeof(float));
}

int calculate_distance(in_char emb1[EMBEDDING_SIZE],
                       in_char emb2[EMBEDDING_SIZE], char dist[4]) {

  // int emb_len1 = unseal_data_inplace(emb1, EMBEDDING_SIZE);
  // int emb_len2 = unseal_data_inplace(emb2, EMBEDDING_SIZE);

  // TEE_ASSERT(emb_len1 == emb_len2, "EMB1 LEN: %d, EMB2 LEN: %d\n", emb_len1,
  //            emb_len2);

  // emb_len1 /= sizeof(float);

  float *f1 = (float *)emb1;
  float *f2 = (float *)emb2;

  float sum = 0.f;
  for (int i = 0; i < EMB_LEN; i++) {
    float d = f1[i] - f2[i];
    sum += d * d;
  }

  memcpy(dist, &sum, sizeof(float));
  return 0;
}
