#include <Arduino.h>
#include <WiFi.h>
#include "global.h"
#include "monitor_OTA.h"
#include "serial_handle.h"
#include "monitor_sensors.h"
#include "fan_control.h"
#include "IR_reciever.h"
#include "LCD_screen.h"

// -------- Global Variables --------
int fan_speed = 0;
int light_intensity = 0;
int latest_light = 0;
int motion_state = 0;
float latest_temp = 0.0f;
float latest_humidity = 0.0f;
bool fanState = false;

// LED override and intensity
bool ledOverride = false;
float ledIntensity = 0;


#define NEO_GPIO 45
#define NEO_NUM 1
Adafruit_NeoPixel strip(NEO_NUM, NEO_GPIO, NEO_GRB + NEO_KHZ800);

SemaphoreHandle_t lcdMutex;

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("Hello World!");

    // Setup LED PWM
    ledcSetup(0, 5000, 12); 
    ledcAttachPin(LED_GPIO, 0);
    ledcWrite(0, 0);

    // Setup NeoPixel
    strip.begin();
    strip.show(); // All pixels off initially

    lcdMutex = xSemaphoreCreateMutex();

    xTaskCreate(handle_LCD, "LCD Task", 4096, NULL, 1, NULL);
    xTaskCreate(monitor_OTA, "Monitor OTA", 4096, NULL, 3, NULL);

    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));
        Serial.println("Reattempting to connect to WiFi");
    }

    xTaskCreate(handle_serial, "Handle Serial", 8192, NULL, 2, NULL);

    xTaskCreate(monitor_light, "Monitor Light", 4096, NULL, 2, NULL);
    xTaskCreate(monitor_motion, "Monitor Motion", 4096, NULL, 2, NULL);
    xTaskCreate(monitor_temp_humidity, "Monitor Temp/Hum", 4096, NULL, 2, NULL);

    xTaskCreate(fan_control, "Fan Control", 4096, NULL, 2, NULL);
    xTaskCreate(handle_IR, "IR Receiver", 4096, NULL, 2, NULL);
}

void loop() {}
