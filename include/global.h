#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Adafruit_NeoPixel.h>
#include "freertos/FreeRTOS.h"

int AI_enabled;
int LCD_enabled = 1;

// fan speed variable 0-255
int fan_speed;

// light intensity variable 0-1500 corresponding to 0-4095
int light_intensity;
float temperature;
float humidity;

int motion_detected;

// queue to store incoming remote control commands
QueueHandle_t irQueue;


#endif