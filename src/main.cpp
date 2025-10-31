#include <Arduino.h>
#include "global.h"
#include "monitor_OTA.h"
#include "serial_handle.h"
#include "monitor_sensors.h"
#include "fan_control.h"

extern int fan_speed;

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("Hello World!");
  Wire.begin(11, 12);

  xTaskCreate(monitor_OTA, "Monitor OTA", 4096, NULL, 3, NULL);


  while (WiFi.status() != WL_CONNECTED)
  {
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
    {
        Serial.println("Access Point mode active");
        break;
    }
    vTaskDelay(pdMS_TO_TICKS(2100));
    Serial.println("Reattempting to connect to WiFi");
  }
  

  xTaskCreate(handle_serial, "Handle Serial", 8192, NULL, 4, NULL);

  xTaskCreate(monitor_sensors, "Monitor sensors", 4096, NULL, 2, NULL);

  // xTaskCreate(fan_control, "Fan Control", 4096, NULL, 2, NULL);
  
}


void loop(){}
