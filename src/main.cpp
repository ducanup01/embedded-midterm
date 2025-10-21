#include <Arduino.h>
#include "monitor_OTA.h"

void setup()
{
  Serial.begin(115200);
  delay(3000);
  Serial.println("Hello World!");


  
  xTaskCreate(monitor_OTA, "Monitor OTA", 2048, NULL, 2, NULL);
}

void loop(){}
