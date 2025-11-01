#ifndef __TINYML__
#define __TINYML__

#include <Arduino.h>
#include "embedded_AI.h"

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#define FAN_PIN GPIO_NUM_6
#define FAN_PWM_CHANNEL 1
#define FAN_PWM_FREQ 25000
#define FAN_PWM_RESOLUTION 8

extern int light_intensity;
extern float temperature;
extern float humidity;
extern int motion_detected;
extern int fan_speed;
extern int AI_enabled;

namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* input = nullptr;
  TfLiteTensor* output = nullptr;

  constexpr int kTensorArenaSize = 8 * 1024;
  uint8_t tensor_arena[kTensorArenaSize];
}

void tinyML(void *pvParameters)
{
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  model = tflite::GetModel(AI_powered_fan);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("Model schema %d not equal to supported %d",
                           model->version(), TFLITE_SCHEMA_VERSION);
    while (1);
  }

  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    while (1);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

    while (1)
    {
        // === 1. Normalize / scale input data ===
        float brightness_scaled = light_intensity / 1500.0f;  // assuming 0–1500 range
        float temperature_scaled = temperature / 100.0f;      // assuming 0–100 °C
        float humidity_scaled = humidity / 100.0f;            // assuming 0–100 %
        float motion_scaled = motion_detected ? 1.0f : 0.0f;  // binary

        // === 2. Feed input tensor ===
        input->data.f[0] = brightness_scaled;
        input->data.f[1] = temperature_scaled;
        input->data.f[2] = humidity_scaled;
        input->data.f[3] = motion_scaled;

        // === 3. Run inference ===
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
        error_reporter->Report("Invoke failed");
        continue;
        }

        // === 4. Get output ===
        if (AI_enabled)
        {
            float fan_output = output->data.f[0];  // assuming single float output (0–1)
            fan_speed = (int)(fan_output * 255);   // map 0–1 → 0–255 for PWM
        }

        vTaskDelay(pdMS_TO_TICKS(100));

    }

}




#endif


