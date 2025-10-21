#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Adafruit_NeoPixel.h>
#define LED_GPIO 48  // ESP32 pin for LED
#define NEO_GPIO 45
#define NEO_NUM 1

// Fan control variable (0–255)
extern int fan_speed;

// PIR motion state (0/1)
extern int motion_state;

// Latest DHT20 readings
extern float latest_temp;
extern float latest_humidity;

// Fan state controlled by IR
extern bool fanState;

// NeoPixel state controlled by IR
extern bool neoState;

// NeoPixel strip object
extern Adafruit_NeoPixel strip;

#endif
