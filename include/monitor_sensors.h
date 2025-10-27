#ifndef __MONITOR_SENSORS_
#define __MONITOR_SENSORS_

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LDR_PIN GPIO_NUM_3

extern int light_intensity;

// QueueHandle_t serialTxQueue;
// SemaphoreHandle_t serialMutex;

void monitor_light(void *pvParameters)
{
    pinMode(LDR_PIN, INPUT);
    Adafruit_NeoPixel strip(1, GPIO_NUM_45, NEO_GRB + NEO_KHZ800);

    while (1)
    {
        light_intensity = analogRead(LDR_PIN);
        // Serial.printf("[Light] Intensity is %d\n\r", light_intensity);

        if (light_intensity < 2048)
        {
            // Serial.print("[Light] TURN OFF NOW");
            strip.setPixelColor(0, strip.Color(0, 0, 0));
            strip.show();
        //     if (eTaskGetState(neoHandle) != eSuspended)
        //     {
        //         vTaskSuspend(neoHandle);
        //     }
        // }
        // else
        // {
        //     Serial.print("[Light] ok you can turn on");
        //     vTaskResume(neoHandle);
        }
        else
        {
            strip.setPixelColor(0, strip.Color(255, 255, 255));
            strip.show();            
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#endif