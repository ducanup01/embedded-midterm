#ifndef __MONITOR_OTA__
#define __MONITOR_OTA__
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#define BOOT_BTN GPIO_NUM_0

const char* ssid = "Fulbright_Student1";
const char* password = "fulbright2018";
const uint16_t TCP_PORT = 69;


const char* ap_ssid = "YOLOUNO 101";
const char* ap_password = "123123123";

WiFiServer server(TCP_PORT);
WiFiClient client;

void handle_serial_input()
{
    if (!client || !client.connected())
    {
        client = server.available();
        if (client)
        {
            Serial.println("Client connected!");
            client.println("Connected to ESP32 WiFi Serial Bridge!");
        }
    }

    if (Serial.available())
    {
        while (Serial.available())
        {
            client.write(Serial.read());
            // ArduinoOTA.handle();
        }
    }

    if (client && client.available())
    {
        String input = client.readStringUntil('\n');
        input.trim();

        if (input.length() > 0)
        {
            int speedValue = input.toInt();

            if (speedValue == 0)
            {
                client.println("Motors stopped!");
            }
            else
            {
                client.printf("Motors running at %d (PWM duty)\n", speedValue);
            }
        }
    }
}

void switchToAPMode()
{
    Serial.println("Switching to AP mode for 5 minutes...");
    vTaskDelay(pdMS_TO_TICKS(50));
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);

    if (WiFi.softAP(ap_ssid, ap_password))
    {
        Serial.println("Access Point started!");
        vTaskDelay(pdMS_TO_TICKS(50));
        Serial.print("SSID: "); vTaskDelay(pdMS_TO_TICKS(50));
        Serial.println(ap_ssid); vTaskDelay(pdMS_TO_TICKS(50));
        Serial.print("Password: "); vTaskDelay(pdMS_TO_TICKS(50));
        Serial.println(ap_password); vTaskDelay(pdMS_TO_TICKS(50));
        Serial.print("AP IP address: "); vTaskDelay(pdMS_TO_TICKS(50));
        Serial.println(WiFi.softAPIP()); vTaskDelay(pdMS_TO_TICKS(50));
    }
    else
    {
        Serial.println("Failed to start AP mode!"); vTaskDelay(pdMS_TO_TICKS(50));
        return;
    }

    ArduinoOTA.begin();
    unsigned long apStart = millis();

    while (millis() - apStart < 5*60*1000UL)
    {
        ArduinoOTA.handle();
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    Serial.println("\n5 minutes passed, returning to STA mode..."); vTaskDelay(pdMS_TO_TICKS(50));
    WiFi.softAPdisconnect(true);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("Reconnecting to WiFi..."); vTaskDelay(pdMS_TO_TICKS(50));

    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.println("WiFi failed, retrying...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    Serial.print("WiFi connected: "); vTaskDelay(pdMS_TO_TICKS(50));
    Serial.println(ssid); vTaskDelay(pdMS_TO_TICKS(50));
    Serial.print("IP address: "); vTaskDelay(pdMS_TO_TICKS(50));
    Serial.println(WiFi.localIP()); vTaskDelay(pdMS_TO_TICKS(50));

    ArduinoOTA.begin();

}

void monitor_OTA(void *pvParameters)
{
    Serial.println("Booting..."); vTaskDelay(pdMS_TO_TICKS(50));

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi..."); vTaskDelay(pdMS_TO_TICKS(50));

    int retryCount = 0;

    while (WiFi.status() != WL_CONNECTED && retryCount < 3)
    {
        Serial.println("WiFi not connected, retrying...");
        WiFi.disconnect();
        WiFi.begin(ssid, password);
        retryCount++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("No WiFi connection. Switching to AP mode...");
        switchToAPMode();
    }

    Serial.println("WiFi connected!"); vTaskDelay(pdMS_TO_TICKS(50));

    ArduinoOTA.setHostname("YOLOUNO101");
    ArduinoOTA.setPassword("123123123");

    ArduinoOTA
        .onStart([]() 
        {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
            else
            type = "filesystem";

            Serial.println("Start updating " + type);
        })
        .onEnd([]() 
        {
            Serial.println("\nUpdate complete!");
        })
        .onProgress([](unsigned int progress, unsigned int total) 
        {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });
    
    ArduinoOTA.begin();

    Serial.println("Ready for OTA updates!"); vTaskDelay(pdMS_TO_TICKS(50));
    Serial.print("IP address: "); vTaskDelay(pdMS_TO_TICKS(50));
    Serial.println(WiFi.localIP());

    pinMode(BOOT_BTN, INPUT_PULLUP);

    while (1)
    {
        if (digitalRead(BOOT_BTN) == LOW)
        {
            vTaskDelay(pdMS_TO_TICKS(50));
            if (digitalRead(BOOT_BTN) == LOW)
            {
                while (digitalRead(BOOT_BTN) == LOW)
                {
                    vTaskDelay(pdMS_TO_TICKS(50));
                }

                switchToAPMode();
            }
        }

        ArduinoOTA.handle();

        handle_serial_input();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#endif