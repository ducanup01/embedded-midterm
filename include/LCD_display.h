#ifndef __LCD_DISPLAY__
#define __LCD_DISPLAY__

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD 16x2 at I2C address 0x27 (most common)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void task_displayLCD(void *pvParameters)
{
  lcd.init();
  lcd.backlight();

  while (1)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Hello World!");
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

#endif
