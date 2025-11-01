#ifndef __MONITOR_SENSORS_
#define __MONITOR_SENSORS_

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <DHT20.h>
// #include "freertos/FreeRTOS.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#define LDR_PIN GPIO_NUM_3
#define RECV_PIN GPIO_NUM_8
#define MOTION_PIN GPIO_NUM_18
#define IR_QUEUE_SIZE 5

DHT20 dht20;

extern QueueHandle_t irQueue;

IRrecv irrecv(RECV_PIN);
decode_results results;


extern int light_intensity;
extern float temperature;
extern float humidity;
extern int motion_detected;

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
    default:       return '?';  // unknown code
  }
}

void monitor_dht20()
{
    dht20.read();
    temperature = dht20.getTemperature();
    humidity = dht20.getHumidity();
}
void monitor_light()
{
    light_intensity = analogRead(LDR_PIN);
    light_intensity = map(light_intensity, 0, 4095, 0, 1500);
}
void monitor_IRremote()
{
    if (irrecv.decode(&results))
    {
        if (results.value != 0xFFFFFFFFFFFFFFFF)
        {
            char button_pressed = mapIRCode(results.value);

            xQueueSend(irQueue, &button_pressed, 0);
        }
        irrecv.resume();
    }
}

void monitor_motion()
{
    motion_detected = gpio_get_level(MOTION_PIN) ? 1 : 0;
}

void monitor_sensors(void *pvParameters)
{
    pinMode(LDR_PIN, INPUT);
    pinMode(MOTION_PIN, INPUT);

    irrecv.enableIRIn();


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