#ifndef __FAN_CONTROL__
#define __FAN_CONTROL__

#include <Arduino.h>

#define FAN_PIN GPIO_NUM_6
#define FAN_PWM_CHANNEL 1
#define FAN_PWM_FREQ 25000
#define FAN_PWM_RESOLUTION 8

extern int fan_speed;

void fan_control(void *pvParameters)
{
    ledcSetup(FAN_PWM_CHANNEL, FAN_PWM_FREQ, FAN_PWM_RESOLUTION);
    ledcAttachPin(FAN_PIN, FAN_PWM_CHANNEL);

    while (1)
    {
        ledcWrite(FAN_PWM_CHANNEL, fan_speed);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#endif