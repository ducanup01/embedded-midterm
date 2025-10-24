#ifndef __SERIAL_HANDLE__
#define __SERIAL_HANDLE__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#define FAN_PIN GPIO_NUM_8
#define FAN_PWM_CHANNEL 0       // PWM channel 0
#define FAN_PWM_FREQ 25000      // 25 kHz, common for fans
#define FAN_PWM_RESOLUTION 8    // 8-bit resolution (0-255)

const uint16_t TCP_PORT = 5000;

WiFiServer server(TCP_PORT);
WiFiClient client;

void handle_serial(void *pvParameters)
{
    server.begin();
    Serial.println("Begin handling serial commands");

    ledcSetup(FAN_PWM_CHANNEL, FAN_PWM_FREQ, FAN_PWM_RESOLUTION);
    ledcAttachPin(FAN_PIN, FAN_PWM_CHANNEL);

    for (;;)
    {
        if (Serial.available())
        {
            String cmd = Serial.readStringUntil('\n');
            cmd.trim();
            
            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, cmd);

            if (error) {
                Serial.print("Invalid JSON: ");
                Serial.println(cmd);
                return;
            }

            const char* method = doc["method"];

            if (strcmp(method, "Fan") == 0)
            // if (method == "Fan")
            {
                int percent = doc["params"];
                int duty = map(percent, 0, 100, 0, 255);
                ledcWrite(FAN_PWM_CHANNEL, duty);

                // Serial.print("Fan set to ");
                // Serial.print(percent);
                // Serial.print("% -> PWM duty ");
                // Serial.println(duty);
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