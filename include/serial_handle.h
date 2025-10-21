#ifndef __SERIAL_HANDLE__
#define __SERIAL_HANDLE__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

extern int fan_speed;
extern int light_intensity;
extern int motion_state;
extern float latest_temp;
extern float latest_humidity;
extern bool fanState;

int update_delay = 1000;
SemaphoreHandle_t serialMutex;

void read_from_rasp()
{
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    StaticJsonDocument<200> raspDoc;
    if (deserializeJson(raspDoc, cmd))
        return;

    const char* method = raspDoc["method"];
    if (!method) return;

    if (strcmp(method, "Fan") == 0)
    {
        int percent = raspDoc["params"];
        fan_speed = map(percent, 0, 100, 0, 255);

        if (percent > 0) fanState = true;
        else fanState = false;
    }
}

void handle_serial(void *pvParameters)
{
    Serial.println("Begin handling serial commands");

    serialMutex = xSemaphoreCreateMutex();
    unsigned long lastSendTime = 0;

    while (1)
    {
        if (Serial.available())
        {
            if (xSemaphoreTake(serialMutex, portMAX_DELAY))
            {
                read_from_rasp();
                xSemaphoreGive(serialMutex);
            }
        }

        unsigned long now = millis();
        if (now - lastSendTime > update_delay)
        {
            if (xSemaphoreTake(serialMutex, portMAX_DELAY))
            {
                StaticJsonDocument<256> espDoc;

                espDoc["brightness"] = light_intensity;
                espDoc["motion"] = motion_state;
                espDoc["temp"] = latest_temp;
                espDoc["humidity"] = latest_humidity;
                espDoc["fan"] = fanState ? fan_speed : 0;

                char buffer[128];
                serializeJson(espDoc, buffer, sizeof(buffer));
                Serial.println(buffer);

                lastSendTime = now;
                xSemaphoreGive(serialMutex);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#endif
