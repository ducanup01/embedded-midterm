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

// -----------------------------------------------------------------------------
// PWM configuration for fan control
// -----------------------------------------------------------------------------
#define FAN_PIN GPIO_NUM_6
#define FAN_PWM_CHANNEL 1
#define FAN_PWM_FREQ 25000
#define FAN_PWM_RESOLUTION 8

// -----------------------------------------------------------------------------
// External variables shared across system modules
// -----------------------------------------------------------------------------
extern float light_intensity;
extern float temperature;
extern float humidity;
extern int motion_detected;
extern int fan_speed;
extern int AI_enabled;
extern SemaphoreHandle_t sensorMutex;

// -----------------------------------------------------------------------------
// TensorFlow Lite Micro objects and buffers
// -----------------------------------------------------------------------------
namespace {
  tflite::ErrorReporter* error_reporter = nullptr;       // For logging errors
  const tflite::Model* model = nullptr;                  // Pointer to the TFLite model
  tflite::MicroInterpreter* interpreter = nullptr;       // TFLite interpreter
  TfLiteTensor* input = nullptr;                         // Model input tensor
  TfLiteTensor* output = nullptr;                        // Model output tensor

  constexpr int kTensorArenaSize = 8 * 1024;             // Memory arena size (8 KB)
  uint8_t tensor_arena[kTensorArenaSize];                // Memory arena for model tensors
}

// -----------------------------------------------------------------------------
// @brief Task function running a TinyML model to control the fan speed
//        based on real-time sensor data (brightness, temperature, humidity, motion).
// @param pvParameters FreeRTOS task parameter (unused)
// -----------------------------------------------------------------------------
void tinyML(void *pvParameters)
{
  // --- Initialize error reporter ---
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // --- Load the TensorFlow Lite model ---
  model = tflite::GetModel(AI_powered_fan);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("Model schema %d not equal to supported %d",
                           model->version(), TFLITE_SCHEMA_VERSION);
    while (1);
  }

  // --- Create resolver and interpreter ---
  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // --- Allocate memory for model tensors ---
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    while (1);
  }

  // --- Get model input and output references ---
  input = interpreter->input(0);
  output = interpreter->output(0);

  // --- Main inference loop ---
  while (1)
  {
    if (xSemaphoreTake(sensorMutex, portMAX_DELAY))
    {
        // ============================================================
        // 1. Normalize / scale input data to 0–1 range
        // ============================================================
        float brightness_scaled = light_intensity / 1500.0f;  // Scale brightness (0–1500 → 0–1)
        float temperature_scaled = temperature / 100.0f;      // Scale temperature (0–100°C → 0–1)
        float humidity_scaled = humidity / 100.0f;            // Scale humidity (0–100% → 0–1)
        float motion_scaled = motion_detected ? 1.0f : 0.0f;  // Binary motion value (0 or 1)
    
        // ============================================================
        // 2. Feed input tensor
        // ============================================================
        input->data.f[0] = brightness_scaled;
        input->data.f[1] = temperature_scaled;
        input->data.f[2] = humidity_scaled;
        input->data.f[3] = motion_scaled;
        xSemaphoreGive(sensorMutex);
    }

    // ============================================================
    // 3. Run inference
    // ============================================================
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
      error_reporter->Report("Invoke failed");
      continue;
    }

    // ============================================================
    // 4. Retrieve output and apply control logic
    // ============================================================
    if (AI_enabled)
    {
      float fan_output = output->data.f[0];    // Model output (float between 0–1)
      if (xSemaphoreTake(sensorMutex, portMAX_DELAY))
      {
        fan_speed = (int)(fan_output * 255);     // Convert to PWM duty cycle (0–255)
        xSemaphoreGive(sensorMutex);
      }
    }

    // ============================================================
    // 5. Delay between inference cycles
    // ============================================================
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

#endif
