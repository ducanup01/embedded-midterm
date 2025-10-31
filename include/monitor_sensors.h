#ifndef __MONITOR_SENSORS_
#define __MONITOR_SENSORS_

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
// #include <LiquidCrystal_I2C.h>
#include <DHT20.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#define LDR_PIN GPIO_NUM_3
#define RECV_PIN GPIO_NUM_8

// LiquidCrystal_I2C lcd(33, 16, 2);
DHT20 dht20;

IRrecv irrecv(RECV_PIN);
decode_results results;


extern int light_intensity;
extern float temperature;
extern float humidity;

void monitor_dht20()
{
    dht20.read();
    temperature = dht20.getTemperature();
    humidity = dht20.getHumidity();

    // Serial.printf("Temp=%.2f Hum=%.2f\r\n", temperature, humidity);

}
void monitor_light()
{
    light_intensity = analogRead(LDR_PIN);

    // Serial.printf("Light=%d\r\n", light_intensity);
}
void monitor_IRremote()
{
    if (irrecv.decode(&results))
    {
        if (results.value != 0xFFFFFFFFFFFFFFFF)
        {
        Serial.printf("Received IR code: %08X\n", results.value);  // Print the code in HEX
        }
        irrecv.resume();  // Receive the next value
    }
}

void monitor_sensors(void *pvParameters)
{
    pinMode(LDR_PIN, INPUT);

    irrecv.enableIRIn();

    Adafruit_NeoPixel strip(1, GPIO_NUM_45, NEO_GRB + NEO_KHZ800);

    while (1)
    {
        monitor_dht20();
        monitor_light();
        // monitor_IRremote();


        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

#endif