#include <Arduino.h>
#include "monitor_OTA.h"
#include "serial_handle.h"

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("Hello World!");

  xTaskCreate(monitor_OTA, "Monitor OTA", 4096, NULL, 3, NULL);
  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(pdMS_TO_TICKS(2000));
    Serial.println("Reattempting to connect to WiFi");
  }
  

  xTaskCreate(handle_serial, "Handle Serial", 8192, NULL, 2, NULL);
}


void loop(){}
