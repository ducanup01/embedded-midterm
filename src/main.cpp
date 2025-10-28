#include <Arduino.h>
#include "monitor_OTA.h"
#include "LCD_display.h"

void setup()
{
  Serial.begin(115200);
  delay(3000);
  Serial.println("Hello World!");

  // Initialize I2C pins for LCD
  Wire.begin(11, 12); // SDA = 11, SCL = 12 (adjust if needed)

  // Create background tasks
  xTaskCreate(monitor_OTA, "Monitor OTA", 4096, NULL, 2, NULL);
  xTaskCreate(task_displayLCD, "Display LCD", 2048, NULL, 1, NULL);
}

void loop() {}
