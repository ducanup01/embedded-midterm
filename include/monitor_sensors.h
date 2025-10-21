#ifndef __MONITOR_SENSORS_
#define __MONITOR_SENSORS_

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <DHT20.h>

extern int light_intensity;
extern int motion_state;
extern float latest_temp;
extern float latest_humidity;
extern bool neoState; // tells compiler this variable exists elsewhere

#define LDR_PIN GPIO_NUM_3
#define PIR_PIN GPIO_NUM_1
#define DHT20_SDA 11
#define DHT20_SCL 12

DHT20 dht20;
extern Adafruit_NeoPixel strip;

// ---------------- LIGHT SENSOR MONITOR ----------------
void monitor_light(void *pvParameters)
{
    pinMode(LDR_PIN, INPUT);
    strip.begin();

    while (1)
    {
        light_intensity = analogRead(LDR_PIN);

        // IR toggle has priority
        if (neoState) {
            strip.setPixelColor(0, strip.Color(255, 255, 255)); // ON
        } else {
            // Follow light sensor
            if (light_intensity < 2000)
                strip.setPixelColor(0, strip.Color(255, 255, 255)); // ON
            else
                strip.setPixelColor(0, strip.Color(0, 0, 0));       // OFF
        }

        strip.show();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}


// ---------------- MOTION SENSOR MONITOR ----------------
void monitor_motion(void *pvParameters)
{
    pinMode(PIR_PIN, INPUT);

    while (1)
    {
        motion_state = digitalRead(PIR_PIN);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// ---------------- TEMPERATURE / HUMIDITY MONITOR ----------------
void monitor_temp_humidity(void *pvParameters)
{
    Wire.begin(DHT20_SDA, DHT20_SCL);
    if (!dht20.begin()) {
        Serial.println("DHT20 initialization failed!");
        while (1) delay(1000);
    }
    delay(100);

    while (1)
    {
        if (dht20.read())
        {
            latest_temp = dht20.getTemperature();
            latest_humidity = dht20.getHumidity();
        }
        else
        {
            Serial.println("DHT20 read failed!");
        }

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

#endif
