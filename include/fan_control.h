#ifndef __FAN_CONTROL__
#define __FAN_CONTROL__

#include <Arduino.h>
#include "global.h"

#define FAN_PIN GPIO_NUM_4
#define FAN_PWM_CHANNEL 1
#define FAN_PWM_FREQ 25000
#define FAN_PWM_RESOLUTION 8

void fan_control(void *pvParameters)
{
    ledcSetup(FAN_PWM_CHANNEL, FAN_PWM_FREQ, FAN_PWM_RESOLUTION);
    ledcAttachPin(FAN_PIN, FAN_PWM_CHANNEL);

    while (1)
    {
        // Write PWM value based on fanState and fan_speed
        ledcWrite(FAN_PWM_CHANNEL, fanState ? fan_speed : 0);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#endif
