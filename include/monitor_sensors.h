#ifndef __MONITOR_SENSORS_
#define __MONITOR_SENSORS_

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <DHT20.h>
// #include "freertos/FreeRTOS.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

/// @brief Analog pin connected to the LDR sensor
#define LDR_PIN GPIO_NUM_3

/// @brief Pin connected to the IR receiver module
#define RECV_PIN GPIO_NUM_8

/// @brief Digital pin connected to the motion sensor (PIR)
#define MOTION_PIN GPIO_NUM_18

/// @brief Maximum number of items in the IR command queue
#define IR_QUEUE_SIZE 5

/// @brief DHT20 temperature and humidity sensor instance
DHT20 dht20;

/// @brief Queue handle for storing incoming IR remote commands
extern QueueHandle_t irQueue;

/// @brief IR receiver instance
IRrecv irrecv(RECV_PIN);

/// @brief Structure to store decoded IR results
decode_results results;

/// @brief Global variable for light intensity (0–1500 mapped from 0–4095 ADC)
extern int light_intensity;

/// @brief Global variable for temperature (°C)
extern float temperature;

/// @brief Global variable for humidity (%)
extern float humidity;

/// @brief Global flag indicating motion detection (1 = detected, 0 = none)
extern int motion_detected;

/**
 * @brief Maps an IR remote control code to a corresponding character.
 * 
 * @param code The received IR code.
 * @return The mapped character representing the pressed button, or '?' if unknown.
 */
char mapIRCode(unsigned long long code)
{
  switch (code) {
    case 0xFFA25D: return 'A';
    case 0xFF629D: return 'B';
    case 0xFFE21D: return 'C';
    case 0xFF22DD: return 'D';
    case 0xFF02FD: return '^';
    case 0xFFC23D: return 'E';
    case 0xFFE01F: return '<';
    case 0xFFA857: return 'w';
    case 0xFF906F: return '>';
    case 0xFF6897: return '0';
    case 0xFF9867: return 'v';
    case 0xFFB04F: return 'F';
    case 0xFF30CF: return '1';
    case 0xFF18E7: return '2';
    case 0xFF7A85: return '3';
    case 0xFF10EF: return '4';
    case 0xFF38C7: return '5';
    case 0xFF5AA5: return '6';
    case 0xFF42BD: return '7';
    case 0xFF4AB5: return '8';
    case 0xFF52AD: return '9';
    default:       return '?';  // Unknown code
  }
}

/**
 * @brief Reads temperature and humidity data from the DHT20 sensor.
 */
void monitor_dht20()
{
    dht20.read();
    temperature = dht20.getTemperature();
    humidity = dht20.getHumidity();
}

/**
 * @brief Reads the light intensity value from the LDR sensor and maps it to 0–1500 range.
 */
void monitor_light()
{
    light_intensity = analogRead(LDR_PIN);
    light_intensity = map(light_intensity, 0, 4095, 0, 1500);
}

/**
 * @brief Handles IR remote input and pushes decoded commands into a FreeRTOS queue.
 */
void monitor_IRremote()
{
    if (irrecv.decode(&results))
    {
        if (results.value != 0xFFFFFFFFFFFFFFFF)
        {
            char button_pressed = mapIRCode(results.value);
            xQueueSend(irQueue, &button_pressed, 0);
        }
        irrecv.resume(); // Prepare for the next IR signal
    }
}

/**
 * @brief Monitors the motion sensor and updates the motion_detected flag.
 */
void monitor_motion()
{
    motion_detected = gpio_get_level(MOTION_PIN) ? 1 : 0;
}

/**
 * @brief Main task that periodically reads all sensor data.
 * 
 * This task continuously:
 * - Reads temperature and humidity from DHT20
 * - Reads light intensity from LDR
 * - Detects motion from PIR sensor
 * - Handles incoming IR remote signals
 * 
 * The loop executes every 200ms.
 */
void monitor_sensors(void *pvParameters)
{
    pinMode(LDR_PIN, INPUT);
    pinMode(MOTION_PIN, INPUT);

    irrecv.enableIRIn(); // Initialize IR receiver

    Adafruit_NeoPixel strip(1, GPIO_NUM_45, NEO_GRB + NEO_KHZ800);

    while (1)
    {
        monitor_dht20();
        monitor_light();
        monitor_motion();
        monitor_IRremote();

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

#endif
