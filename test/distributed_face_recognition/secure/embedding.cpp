#include "embedding.h"
#include "MobileFaceNet.inc"
#include "tensorflow/lite/builtin_op_data.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"
#include "tensorflow/lite/string_util.h"
#include <cstdio>
#ifdef __TEE
extern "C" void eapp_print(const char *, ...);
#else
#define eapp_print printf
#endif
#define TFLITE_MINIMAL_CHECK(x)                                                \
  if (!(x)) {                                                                  \
    eapp_print("ERROR!\n");                                                    \
  }

TfLiteStatus InvokeNode(tflite::Subgraph& g) {
  const int N = g.execution_plan().size();
  auto ctx = g.context();
  // for (int i = 0; i < g.tensors_size(); i++) {
  //   eapp_print(":: %p\n", g.tensor(i)->data.raw);
  // }
  
  for (int execution_plan_index = 0;
       execution_plan_index < N; execution_plan_index++) {
    eapp_print("--- %d/%d\n", execution_plan_index, N);
    int node_index = g.execution_plan()[execution_plan_index];
    eapp_print("A\n");
    TfLiteNode& node = const_cast<TfLiteNode&>(g.nodes_and_registration()[node_index].first);
    eapp_print("A\n");
    const TfLiteRegistration& registration =
        g.nodes_and_registration()[node_index].second;
    eapp_print("A %p %p %p\n", registration.invoke, ctx, &node);
    registration.invoke(ctx, &node);
    eapp_print("A\n");
  }
  eapp_print("OK\n");
  return kTfLiteOk;
}

TfLiteStatus MyInvoke(tflite::Interpreter* ip) {
  InvokeNode(ip->primary_subgraph());
  return kTfLiteOk;
}

int embedding(in_char img[IMG_SIZE], out_char res[EMBEDDING_SIZE]) {
  // Load model
  std::unique_ptr<tflite::FlatBufferModel> model =
      tflite::FlatBufferModel::BuildFromBuffer((const char *)MobileFaceNet,
                                               MobileFaceNet_len);
  TFLITE_MINIMAL_CHECK(model != nullptr);
  tflite::ops::builtin::BuiltinOpResolver resolver;
  tflite::InterpreterBuilder builder(*model, resolver);
  std::unique_ptr<tflite::Interpreter> interpreter;
  builder(&interpreter);
  TFLITE_MINIMAL_CHECK(interpreter != nullptr);

  // Allocate tensor buffers.
  eapp_print("OK\n");
  TFLITE_MINIMAL_CHECK(interpreter->AllocateTensors() == kTfLiteOk);
  eapp_print("OK\n");

  TfLiteTensor *input_tensor = interpreter->tensor(interpreter->inputs()[0]);
//   if (input_tensor->dims->size != 4 || input_tensor->dims->data[0] != 1 ||
//       input_tensor->dims->data[1] != 112 ||
//       input_tensor->dims->data[2] != 112 || input_tensor->dims->data[3] != 3) {
//     std::cerr << "Input tensor has incorrect dimensions." << std::endl;
//     // Handle error...
//   }
  float *input_data = input_tensor->data.f;
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      for (int k = 0; k < 3; k++) {
        int idx = i * HEIGHT * 3 + j * 3 + k;
        input_data[idx] = img[idx] / 255.0f;
      }
    }
  }
  //   memcpy(input_tensor->data.f, input_data, sizeof(input_data));

  eapp_print("OK\n");
  // TFLITE_MINIMAL_CHECK(interpreter->Invoke() == kTfLiteOk);
  TFLITE_MINIMAL_CHECK(MyInvoke(interpreter.get()) == kTfLiteOk);
  eapp_print("OK\n");

  TfLiteTensor *output_tensor = interpreter->tensor(interpreter->outputs()[0]);
  // int len = output_tensor->bytes / sizeof(float);
  // eapp_print("LEN: %d\n", len);
  float* out = (float*)res;
  for (int i = 0; i < EMB_LEN; i++) {
    out[i] = output_tensor->data.f[i];
  }

  return 0;
}
