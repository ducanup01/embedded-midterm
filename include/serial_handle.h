#ifndef __SERIAL_HANDLE__
#define __SERIAL_HANDLE__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// const uint16_t TCP_PORT = 5000;

extern int fan_speed;
extern int light_intensity;
int update_delay = 1000;


SemaphoreHandle_t serialMutex;

void read_from_rasp()
{
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    StaticJsonDocument<200> raspDoc;
    DeserializationError error = deserializeJson(raspDoc, cmd);
    
    if (error) {
        Serial.print("Invalid JSON: ");
        Serial.println(cmd);
        Serial.flush();
        return;
    }
    
    const char* method = raspDoc["method"];
    
    if (strcmp(method, "Fan") == 0)
    // if (method == "Fan")
    {
        int percent = raspDoc["params"];
        fan_speed = map(percent, 0, 100, 0, 255);
    }
}

void handle_serial(void *pvParameters)
{
    // server.begin();
    Serial.println("Begin handling serial commands");

    serialMutex = xSemaphoreCreateMutex();
    
    // serialTxQueue = xQueueCreate(10, sizeof(String));
    // serialMutex = xSemaphoreCreateMutex();
    unsigned long lastSendTime = 0;
    
    while (1)
    {
        if (Serial.available())
        {
            if (xSemaphoreTake(serialMutex, portMAX_DELAY))
            {
                read_from_rasp();
                vTaskDelay(pdMS_TO_TICKS(50));
                xSemaphoreGive(serialMutex);
            }
        }
        
        unsigned long now = millis();
        
        if (now - lastSendTime > update_delay)
        {
            if (xSemaphoreTake(serialMutex, portMAX_DELAY))
            {
                StaticJsonDocument<200> espDoc;
                espDoc["brightness"] = light_intensity;


                char buffer[64];
                serializeJson(espDoc, buffer, sizeof(buffer));
                Serial.println(buffer);
                Serial.flush();
                lastSendTime = now;
    
                xSemaphoreGive(serialMutex);

            }

        }

        // if (!client || !client.connected())
        // {
        //     client = server.available();
        //     if (client)
        //     {
        //         Serial.println("[WiFi Serial Bridge] Client connected");
        //         client.println("Connected to ESP32 WiFi Serial Bridge!");
        //     }
        // }
    
    
        // if (client && client.connected() && client.available())
        // {
        //     String input = client.readStringUntil('\n');
        //     input.trim();
        //     if (input.length() > 0)
        //     {
        //         Serial.printf("[WiFi->USB] %s\n", input.c_str());
        //     }
        // }
    
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#endif