#ifndef __SERIAL_HANDLE__
#define __SERIAL_HANDLE__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

extern int fan_speed;
extern int light_intensity;
extern float temperature;
extern float humidity;
extern int motion_detected;
extern int AI_enabled;
extern int LCD_enabled;


int update_delay = 500;

SemaphoreHandle_t serialMutex;
extern QueueHandle_t irQueue;

float round2(float value) {
    return round(value * 100) / 100.0;
}

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
    {
        int percent = raspDoc["params"];
        fan_speed = map(percent, 0, 100, 0, 255);
    }

    if (strcmp(method, "AI_enabled") == 0)
    {
        AI_enabled = raspDoc["params"];
        if (AI_enabled == 0)
        {
            fan_speed = 0;
        }
    }
}

void handle_serial(void *pvParameters)
{
    Serial.println("Begin handling serial commands");

    // mutex semaphore to ensure smooth 2 way communication between esp32 and rasp 
    serialMutex = xSemaphoreCreateMutex();
    
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
                StaticJsonDocument<1024> espDoc;
                
                espDoc["brightness"] = light_intensity;
                espDoc["temperature"] = round2(temperature); // round to 2 decimals
                espDoc["humidity"] = round2(humidity);
                espDoc["motion_detected"] = motion_detected;
                espDoc["remote"] = "";
                
                char receivedCode;
                if (xQueueReceive(irQueue, &receivedCode, 0) == pdPASS)
                {
                    char remoteStr[2] = { receivedCode, '\0' };
                    espDoc["remote"] = remoteStr;

                    if (remoteStr[0] >= '0' && remoteStr[0] <= '9')
                    {
                        int digit = remoteStr[0] - '0';
                        fan_speed = (digit * 255) / 9;
                        
                        espDoc["Fan"] = fan_speed;
                    }
                    else if (remoteStr[0] == 'C')
                    {
                        LCD_enabled = !LCD_enabled;
                    }


                    
                }


                char buffer[256];
                serializeJson(espDoc, buffer, sizeof(buffer));
                Serial.println(buffer);
                Serial.flush();
                lastSendTime = now;
    
                xSemaphoreGive(serialMutex);

            }

        }
    
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#endif