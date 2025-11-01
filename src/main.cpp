#include <Arduino.h>
#include "global.h"
#include "monitor_OTA.h"
#include "serial_handle.h"
#include "monitor_sensors.h"
#include "fan_and_led_control.h"
#include "lcd_control.h"
#include "tinyML.h"

extern int fan_speed;

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("Hello World!");
    Wire.begin(11, 12);

    irQueue = xQueueCreate(IR_QUEUE_SIZE, sizeof(char));
    if (irQueue == NULL)
    {
        Serial.println("IR Queue creation failed!");
    }

    xTaskCreate(monitor_OTA, "Monitor OTA", 4096, NULL, 3, NULL);

    // check if wifi is connected then create other tasks
    while (WiFi.status() != WL_CONNECTED)
    {
        if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
        {
            Serial.println("Access Point mode active");
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(2100));
        Serial.println("Reattempting to connect to WiFi");
    }


    xTaskCreate(monitor_sensors, "Monitor sensors", 8192, NULL, 2, NULL);

    xTaskCreate(fan_control, "Fan Control", 4096, NULL, 2, NULL);

    xTaskCreate(lcd_control, "LCD control", 4096, NULL, 2, NULL);

    xTaskCreate(handle_serial, "Handle Serial", 8192, NULL, 4, NULL);

    xTaskCreate(tinyML, "AI POWERED FAN", 4096, NULL, 2, NULL);

    xTaskCreate(led_control, "LED", 2048, NULL, 1, NULL);

    xTaskCreate(neo_control, "NEO", 2048, NULL, 1, NULL);
}


void loop(){}