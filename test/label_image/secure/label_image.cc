#include <cstdio>

#include "image_data.inc"
#include "label_image.h"
#include "labels.inc"
#include "model_data.inc"
#include "tensorflow/lite/builtin_op_data.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"
#include "tensorflow/lite/string_util.h"

// printf("Error at %s:%d\n", __FILE__, __LINE__);
#define TFLITE_MINIMAL_CHECK(x)                                                \
  if (!(x)) {                                                                  \
  }

int get_label() {
  // Load model
  std::unique_ptr<tflite::FlatBufferModel> model =
      tflite::FlatBufferModel::BuildFromBuffer((const char *)imported_model,
                                               imported_model_size);
  TFLITE_MINIMAL_CHECK(model != nullptr);

  // Build the interpreter with the InterpreterBuilder.
  // Note: all Interpreters should be built with the InterpreterBuilder,
  // which allocates memory for the Intrepter and does various set up
  // tasks so that the Interpreter can read the provided model.
  tflite::ops::builtin::BuiltinOpResolver resolver;
  tflite::InterpreterBuilder builder(*model, resolver);
  std::unique_ptr<tflite::Interpreter> interpreter;
  builder(&interpreter);
  TFLITE_MINIMAL_CHECK(interpreter != nullptr);

  // Allocate tensor buffers.
  TFLITE_MINIMAL_CHECK(interpreter->AllocateTensors() == kTfLiteOk);

  {
    TfLiteTensor *input_tensor = interpreter->tensor(interpreter->inputs()[0]);
    switch (input_tensor->type) {
    case kTfLiteFloat32: {
      int input0 = interpreter->inputs()[0];

      TfLiteIntArray *dims = interpreter->tensor(input0)->dims;
      int wanted_height = dims->data[1];
      int wanted_width = dims->data[2];
      int wanted_channels = dims->data[3];
      auto number_of_pixels = wanted_height * wanted_width * wanted_channels;

      uint8_t *input = interpreter->typed_tensor<uint8_t>(input0);
      for (int i = 0; i < number_of_pixels; i++) {
        input[i] = imported_image[i] / 255.f;
      }
      break;
    }
    case kTfLiteUInt8: {
      int input0 = interpreter->inputs()[0];

      TfLiteIntArray *dims = interpreter->tensor(input0)->dims;
      int wanted_height = dims->data[1];
      int wanted_width = dims->data[2];
      int wanted_channels = dims->data[3];
      auto number_of_pixels = wanted_height * wanted_width * wanted_channels;

      uint8_t *input = interpreter->typed_tensor<uint8_t>(input0);
      for (int i = 0; i < number_of_pixels; i++) {
        input[i] = static_cast<uint8_t>(imported_image[i]);
      }
      break;
    }
    }
  }

  // Run inference
  int number_of_warmup_runs = 10;
  for (int i = 0; i < number_of_warmup_runs; i++) {
    TFLITE_MINIMAL_CHECK(interpreter->Invoke() == kTfLiteOk);
  }

  int loop_count = 10;
  for (int i = 0; i < loop_count; i++) {
    TFLITE_MINIMAL_CHECK(interpreter->Invoke() == kTfLiteOk);
  }

  // tflite::PrintInterpreterState(interpreter.get());

  // Read output buffers
  // TODO(user): Insert getting data out code.
  // Note: The buffer of the output tensor with index `i` of type T can
  // be accessed with `T* output = interpreter->typed_output_tensor<T>(i);`
  TfLiteTensor *output_tensor = interpreter->tensor(interpreter->outputs()[0]);

  unsigned int max = output_tensor->data.uint8[0];
  unsigned int index = 0;
  for (int i = 1; i < 1001; i++) {
    if (output_tensor->data.uint8[i] > max) {
      max = output_tensor->data.uint8[i];
      index = i;
    }
  }
  // printf("The input image's index is %d, and label is %s\n", index,
  //        imported_labels[index]);

  return index;
}
