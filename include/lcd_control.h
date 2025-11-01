#ifndef __LCD_CONTROL__
#define __LCD_CONTROL__

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

extern int LCD_enabled;
extern int light_intensity;
extern float temperature;
extern float humidity;
extern int motion_detected;


LiquidCrystal_I2C lcd(39, 16, 2); // 33 instead of 39 if use AIoT's kit

void lcd_control(void *pvParameters)
{
    // lcd.begin(16, 2);
    lcd.init();
    lcd.clear();
    
    while (1)
    {
        if (LCD_enabled == 1)
        {
            lcd.backlight();
            lcd.clear();
            // --- Show temperature and humidity ---
            for (int i = 0; i < 10; i++) // update 10 times before switching
            {
                lcd.setCursor(0, 0);
                lcd.printf("Temp: %.2f%cC", temperature, 223);
                lcd.setCursor(0, 1);
                lcd.printf("RH: %.2f%%", humidity);
                vTaskDelay(pdMS_TO_TICKS(300)); // refresh every 0.3s (10x = 3s total)
            }
    
            lcd.clear();
            // --- Show brightness and motion ---
            for (int i = 0; i < 10; i++)
            {
                lcd.setCursor(0, 0);
                lcd.printf("Light: %d lx", light_intensity);
                lcd.setCursor(0, 1);
                lcd.printf("Motion: %s", motion_detected ? "Yes" : "No");
                vTaskDelay(pdMS_TO_TICKS(300)); // refresh every 0.3s (10x = 3s total)
            }
        }
        else
        {
            lcd.clear();
            lcd.noBacklight();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

#endif