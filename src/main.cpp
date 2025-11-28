#include <Arduino.h>
#include "global.h"
#include "monitor_OTA.h"
#include "serial_handle.h"
#include "monitor_sensors.h"
#include "fan_and_led_control.h"
#include "lcd_control.h"
#include "tinyML.h"

extern SemaphoreHandle_t sensorMutex;

// -----------------------------------------------------------------------------
// @brief System setup function - runs once at startup.
//        Initializes serial communication, I2C, queues, and all FreeRTOS tasks.
// -----------------------------------------------------------------------------
void setup()
{
    // --- Initialize Serial Communication ---
    Serial.begin(115200);
    delay(500);
    Serial.println("Hello World!");

    // --- Initialize I2C Communication (for DHT20, LCD, etc.) ---
    Wire.begin(11, 12);  // SDA = GPIO 11, SCL = GPIO 12

    // --- Create Queue and Mutex Semaphores ---
    irQueue = xQueueCreate(IR_QUEUE_SIZE, sizeof(char));
    if (irQueue == NULL){Serial.println("IR Queue creation failed!");}

    sensorMutex = xSemaphoreCreateMutex();
    if (sensorMutex == NULL){Serial.println("Sensor semaphore creation failed!");}



    // // --- Create OTA Monitoring Task ---
    // xTaskCreate(monitor_OTA, "Monitor OTA", 4096, NULL, 3, NULL);

    // // --- Wait for WiFi Connection or Switch to AP Mode ---
    // while (WiFi.status() != WL_CONNECTED)
    // {
    //     if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
    //     {
    //         Serial.println("Access Point mode active");
    //         break;
    //     }
    //     vTaskDelay(pdMS_TO_TICKS(2100));
    //     Serial.println("Reattempting to connect to WiFi");
    // }

    // --- Create Sensor Monitoring Task ---
    xTaskCreate(monitor_sensors, "Monitor sensors", 8192, NULL, 2, NULL);

    // --- Create Fan Control Task ---
    // xTaskCreate(fan_control, "Fan Control", 4096, NULL, 2, NULL);

    // --- Create LCD Display Control Task ---
    // xTaskCreate(lcd_control, "LCD control", 4096, NULL, 2, NULL);

    // --- Create Serial Communication Handling Task ---
    xTaskCreate(handle_serial, "Handle Serial", 8192, NULL, 3, NULL);

    // --- Create TinyML AI-powered Fan Control Task ---
    // xTaskCreate(tinyML, "AI POWERED FAN", 4096, NULL, 2, NULL);

    // --- Create LED Control Tasks ---
    // xTaskCreate(led_control, "LED", 2048, NULL, 2, NULL);
    // xTaskCreate(neo_control, "NEO", 2048, NULL, 2, NULL);
}

// -----------------------------------------------------------------------------
// @brief Main loop - not used since all logic is handled by FreeRTOS tasks.
// -----------------------------------------------------------------------------
void loop() {}
