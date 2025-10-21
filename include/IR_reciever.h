#ifndef __IR_RECEIVER__
#define __IR_RECEIVER__

#include <Arduino.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Adafruit_NeoPixel.h>
#include "global.h"

#define IR_RECV_PIN GPIO_NUM_2
#define DEBOUNCE_DELAY 300
#define FAN_STEP 51

#define IR_CODE_FAN_TOGGLE 0xFFA25D
#define IR_CODE_FAN_INC    0xFF906F
#define IR_CODE_FAN_DEC    0xFFE01F
#define IR_CODE_TINYML_TOGGLE 0xFFE21D
#define IR_CODE_LED_TOGGLE 0xFF22DD
#define IR_CODE_NEO_TOGGLE 0xFFC23D

IRrecv irrecv(IR_RECV_PIN);
decode_results results;
unsigned long lastPressTime = 0;
uint32_t lastCode = 0;

extern int fan_speed;
extern bool fanState;

bool tinyMLState = false;
bool ledState = false;
bool neoState = false;

void setup_IR() {
    irrecv.enableIRIn();
    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LOW); // Start OFF

    strip.begin();
    strip.show();
}

void blinkFanLimit()
{
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(LED_GPIO, HIGH);
        strip.setPixelColor(0, strip.Color(255, 255, 255));
        strip.show();
        vTaskDelay(pdMS_TO_TICKS(250));

        digitalWrite(LED_GPIO, LOW);
        strip.setPixelColor(0, strip.Color(0, 0, 0));
        strip.show();
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

void handle_IR(void *pvParameters)
{
    setup_IR();

    while (1)
    {
        if (irrecv.decode(&results))
        {
            unsigned long currentTime = millis();

            if (results.value != 0xFFFFFFFF)
            {
                if (results.value != lastCode || currentTime - lastPressTime > DEBOUNCE_DELAY)
                {
                    switch (results.value)
                    {
                        case IR_CODE_FAN_TOGGLE:
                            fanState = !fanState;
                            Serial.println(fanState ? "Fan ON (IR)" : "Fan OFF (IR)");
                            break;

                        case IR_CODE_FAN_INC:
                            if (fan_speed + FAN_STEP > 255)
                            {
                                fan_speed = 255;
                                blinkFanLimit();
                                Serial.println("Fan speed reached MAX = 255");
                            }
                            else
                            {
                                fan_speed += FAN_STEP;
                                Serial.printf("Fan speed increased: %d\n", fan_speed);
                            }
                            break;

                        case IR_CODE_FAN_DEC:
                            if (fan_speed - FAN_STEP < 0)
                            {
                                fan_speed = 0;
                                blinkFanLimit();
                                Serial.println("Fan speed reached MIN = 0");
                            }
                            else
                            {
                                fan_speed -= FAN_STEP;
                                Serial.printf("Fan speed decreased: %d\n", fan_speed);
                            }
                            break;

                        case IR_CODE_TINYML_TOGGLE:
                            tinyMLState = !tinyMLState;
                            Serial.println(tinyMLState ? "TINYML ON" : "TINYML OFF");
                            break;

                        case IR_CODE_LED_TOGGLE:
                            ledState = !ledState;
                            digitalWrite(LED_GPIO, ledState ? HIGH : LOW);
                            Serial.println(ledState ? "LED ON" : "LED OFF");
                            break;

                        case IR_CODE_NEO_TOGGLE:
                            neoState = !neoState;
                            if (neoState)
                                strip.setPixelColor(0, strip.Color(255, 255, 255));
                            else
                                strip.setPixelColor(0, strip.Color(0, 0, 0));
                            strip.show();
                            Serial.println(neoState ? "NEO ON" : "NEO OFF");
                            break;

                        default:
                            Serial.println("NULL");
                            break;
                    }

                    lastPressTime = currentTime;
                    lastCode = results.value;
                }
            }

            irrecv.resume();
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#endif
