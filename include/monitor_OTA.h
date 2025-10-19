#ifndef __MONITOR_OTA__
#define __MONITOR_OTA__
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#define LED_PIN GPIO_NUM_48
#define BOOT_BTN GPIO_NUM_0

const char* ssid = "your_ssid";
const char* password = "your_password";


const char* ap_ssid = "YOLOUNO 101";
const char* ap_password = "123123123";

void switchToAPMode()
{
    Serial.println("Switching to AP mode for 5 minutes..."); vTaskDelay(pdMS_TO_TICKS(50));
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);

    if (WiFi.softAP(ap_ssid, ap_password))
    {
        Serial.println("Access Point started!"); vTaskDelay(pdMS_TO_TICKS(50));
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

        gpio_set_level(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(LED_PIN, 0);
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

    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.println("Rebooting...");
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP.restart();
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

    pinMode(LED_PIN, OUTPUT);
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
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));

        gpio_set_level(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

#endif