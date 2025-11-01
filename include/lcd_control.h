#ifndef __LCD_CONTROL__
#define __LCD_CONTROL__

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

/// @brief Flag to enable or disable the LCD display (1 = enabled, 0 = disabled)
extern int LCD_enabled;

/// @brief Light intensity value (in lux)
extern int light_intensity;

/// @brief Current temperature value (°C)
extern float temperature;

/// @brief Current humidity value (%)
extern float humidity;

/// @brief Motion detection flag (1 = motion detected, 0 = no motion)
extern int motion_detected;

/// @brief Initialize a 16x2 I2C LCD display at address 0x27 (use 0x21 or 33 if on AIoT kit)
LiquidCrystal_I2C lcd(39, 16, 2); // Change 39 to 33 if using AIoT's kit

/// @brief Mutex Semaphore for sensor data
extern SemaphoreHandle_t sensorMutex;

/**
 * @brief LCD control task for displaying sensor data.
 * 
 * This task continuously updates the LCD screen with sensor readings such as:
 * - Temperature and humidity
 * - Light intensity and motion detection status
 * 
 * The display switches between the two sets of information every ~3 seconds.
 * The LCD is turned off when LCD_enabled = 0.
 */
void lcd_control(void *pvParameters)
{
    // Initialize LCD display
    lcd.init();
    lcd.clear();
    
    while (1)
    {
        if (LCD_enabled == 1)
        {
            // Turn on LCD backlight and clear screen
            lcd.backlight();
            lcd.clear();

            // --- Display temperature and humidity for ~3 seconds ---
            for (int i = 0; i < 10; i++) // Update 10 times before switching screen
            {
                if (LCD_enabled == 0) break;  // ✅ instantly exit if disabled

                if (xSemaphoreTake(sensorMutex, portMAX_DELAY))
                {
                    lcd.setCursor(0, 0);
                    lcd.printf("Temp: %.2f%cC", temperature, 223);  // 223 = degree symbol
                    lcd.setCursor(0, 1);
                    lcd.printf("RH: %.2f%%", humidity);             // Display relative humidity
                    xSemaphoreGive(sensorMutex);
                }
                vTaskDelay(pdMS_TO_TICKS(300));                 // Refresh every 0.3s (10x = 3s total)
            }
    
            lcd.clear();

            // --- Display light intensity and motion status for ~3 seconds ---
            for (int i = 0; i < 10; i++)
            {
                if (LCD_enabled == 0) break;  // ✅ instantly exit if disabled

                if (xSemaphoreTake(sensorMutex, portMAX_DELAY))
                {
                    lcd.setCursor(0, 0);
                    lcd.printf("Light: %d lx", light_intensity);     // Display light level in lux
                    lcd.setCursor(0, 1);
                    lcd.printf("Motion: %s", motion_detected ? "Yes" : "No"); // Motion detection status
                    xSemaphoreGive(sensorMutex);
                }
                vTaskDelay(pdMS_TO_TICKS(300));                  // Refresh every 0.3s (10x = 3s total)
            }
        }
        else
        {
            // Turn off LCD display and backlight when disabled
            lcd.clear();
            lcd.noBacklight();
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        // Small delay before next iteration to reduce CPU load
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

#endif
