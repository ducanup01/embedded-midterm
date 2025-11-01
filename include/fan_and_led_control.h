#ifndef __FAN_CONTROL__
#define __FAN_CONTROL__

#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

#define LED_PIN GPIO_NUM_48
#define LED_COUNT 1
#define NEO_PIN GPIO_NUM_45

#define FAN_PIN GPIO_NUM_6
#define FAN_PWM_CHANNEL 1
#define FAN_PWM_FREQ 25000
#define FAN_PWM_RESOLUTION 8

Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRBW + NEO_KHZ800);

extern int fan_speed;

void fan_control()
{
    ledcWrite(FAN_PWM_CHANNEL, fan_speed);
}

void led_control(void *pvParameters)
{
    while (1)
    {
        float h = humidity;
        if (h < 40) h = 40;
        if (h > 80) h = 80;
        int delay_ms = 1000 - (int)((h - 40) * (800.0 / 40.0));

        gpio_set_level(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

void neo_control(void *pvParameters)
{
    strip.begin();
    strip.show();
    
    while (1)
    {
        float t = temperature;
        if (t < 25) t = 25;
        if (t > 30) t = 30;

        // the higher the temperature, the more red the NEO
        // the lower the temperature, the more blue the NEO
        float ratio = (t - 25.0f) / 5.0f;
        int red   = (int)(ratio * 255);
        int blue  = (int)((1.0f - ratio) * 255);
        int green = 0;

        strip.setPixelColor(0, strip.Color(red, green, blue));
        strip.show();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void fan_control(void *pvParameters)
{
    ledcSetup(FAN_PWM_CHANNEL, FAN_PWM_FREQ, FAN_PWM_RESOLUTION);
    ledcAttachPin(FAN_PIN, FAN_PWM_CHANNEL);
    pinMode(LED_PIN, OUTPUT);


    while (1)
    {
        fan_control();


        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#endif