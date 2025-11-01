#ifndef __FAN_CONTROL__
#define __FAN_CONTROL__

#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

/// @brief GPIO pin connected to the indicator LED
#define LED_PIN GPIO_NUM_48

/// @brief Number of NeoPixel LEDs in the strip
#define LED_COUNT 1

/// @brief GPIO pin connected to the NeoPixel LED
#define NEO_PIN GPIO_NUM_45

/// @brief GPIO pin connected to the fan
#define FAN_PIN GPIO_NUM_6

/// @brief PWM channel used to control fan speed
#define FAN_PWM_CHANNEL 1

/// @brief PWM frequency for fan control (in Hz)
#define FAN_PWM_FREQ 25000

/// @brief PWM resolution for fan control (in bits)
#define FAN_PWM_RESOLUTION 8

/// @brief NeoPixel LED strip object (GRBW format, 800kHz signal)
Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRBW + NEO_KHZ800);

/// @brief External variable controlling the fan speed (0–255)
extern int fan_speed;

/**
 * @brief Writes the current fan speed to the PWM channel.
 */
void fan_control()
{
    ledcWrite(FAN_PWM_CHANNEL, fan_speed);
}

/**
 * @brief LED blinking task that adjusts blink rate based on humidity.
 * 
 * The LED blinks faster when humidity is low, and slower when humidity is high.
 * Humidity values are clamped between 40% and 80%.
 */
void led_control(void *pvParameters)
{
    while (1)
    {
        float h = humidity;
        if (h < 40) h = 40;
        if (h > 80) h = 80;

        // Map humidity (40–80%) to delay (1000–200 ms)
        int delay_ms = 1000 - (int)((h - 40) * (800.0 / 40.0));

        gpio_set_level(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

/**
 * @brief NeoPixel control task that changes color based on temperature.
 * 
 * - At 25°C: LED is fully blue.
 * - At 30°C: LED is fully red.
 * - Between 25–30°C: color transitions smoothly from blue to red.
 */
void neo_control(void *pvParameters)
{
    strip.begin();
    strip.show();  // Initialize all pixels to 'off'
    
    while (1)
    {
        float t = temperature;
        if (t < 25) t = 25;
        if (t > 30) t = 30;

        // Compute red-blue ratio based on temperature range
        float ratio = (t - 25.0f) / 5.0f;
        int red   = (int)(ratio * 255);
        int blue  = (int)((1.0f - ratio) * 255);
        int green = 0;

        strip.setPixelColor(0, strip.Color(red, green, blue));
        strip.show();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
 * @brief Fan control task that continuously updates fan PWM output.
 * 
 * Initializes PWM configuration and updates fan speed periodically.
 */
void fan_control(void *pvParameters)
{
    // Configure PWM for fan control
    ledcSetup(FAN_PWM_CHANNEL, FAN_PWM_FREQ, FAN_PWM_RESOLUTION);
    ledcAttachPin(FAN_PIN, FAN_PWM_CHANNEL);

    // Configure LED pin as output
    pinMode(LED_PIN, OUTPUT);

    while (1)
    {
        fan_control();  // Apply the latest fan speed value
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#endif
