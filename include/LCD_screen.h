#ifndef __LCD_SCREEN__
#define __LCD_SCREEN__

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <semphr.h>

#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

#define LCD_REFRESH_MS 1000
#define WARNING_FLASH_MS 500

#define TEMP_ALERT_MIN 30.0f
#define TEMP_CRITICAL_MIN 35.0f
#define HUM_ALERT_MIN 60.0f
#define HUM_CRITICAL_MIN 75.0f

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

extern float latest_temp;
extern float latest_humidity;
extern int latest_light;

extern SemaphoreHandle_t lcdMutex;

void setup_LCD() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
}

int getLCDMode() {
    bool tempAlert = latest_temp >= TEMP_ALERT_MIN;
    bool tempCrit = latest_temp >= TEMP_CRITICAL_MIN;

    bool humAlert = latest_humidity >= HUM_ALERT_MIN;
    bool humCrit = latest_humidity >= HUM_CRITICAL_MIN;

    if (tempCrit || humCrit) return 2; // Critical mode
    if (tempAlert || humAlert) return 1; // Alert mode
    return 0; // Normal
}

void handle_LCD(void *pvParameters) {
    setup_LCD();
    bool showTempScreen = true;

    while (1) {
        if (xSemaphoreTake(lcdMutex, portMAX_DELAY)) {
            int mode = getLCDMode();

            lcd.clear();

            if (showTempScreen) {
                lcd.setCursor(0, 0);
                lcd.printf("Temp: %.2fC", latest_temp);
                lcd.setCursor(0, 1);
                lcd.printf("RH:   %.2f%%", latest_humidity);
            } else {
                lcd.setCursor(0, 0);
                lcd.print("Light intensity:");
                lcd.setCursor(0, 1);
                lcd.printf("%d %%", latest_light);
            }

            if (mode == 0) {
                lcd.backlight();  
            }
            else if (mode == 1) {
                lcd.backlight();  
            }
            else if (mode == 2) {
                lcd.backlight();
                vTaskDelay(pdMS_TO_TICKS(WARNING_FLASH_MS));
                lcd.noBacklight();
                vTaskDelay(pdMS_TO_TICKS(WARNING_FLASH_MS));
            }

            xSemaphoreGive(lcdMutex);
        }

        showTempScreen = !showTempScreen;
        vTaskDelay(pdMS_TO_TICKS(LCD_REFRESH_MS));
    }
}

#endif