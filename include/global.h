#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Adafruit_NeoPixel.h>
#include "freertos/FreeRTOS.h"

/// @brief Flag to enable or disable AI features
int AI_enabled;

/// @brief Flag to enable or disable the LCD display (1 = enabled, 0 = disabled)
int LCD_enabled = 1;

/// @brief Fan speed value (range: 0–255)
int fan_speed;

/// @brief Light intensity value (mapped from 0–4095 to 0–1500)
int light_intensity;

/// @brief Measured temperature value (°C)
float temperature;

/// @brief Measured humidity value (%)
float humidity;

/// @brief Motion detection flag (1 = motion detected, 0 = no motion)
int motion_detected;

/// @brief Queue handle for storing incoming IR remote control commands
QueueHandle_t irQueue;

#endif
