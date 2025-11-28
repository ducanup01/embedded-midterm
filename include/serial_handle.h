#ifndef __SERIAL_HANDLE__
#define __SERIAL_HANDLE__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// -----------------------------------------------------------------------------
// External global variables
// -----------------------------------------------------------------------------
extern int fan_speed;
extern float light_intensity;
extern float temperature;
extern float humidity;
extern int motion_detected;
extern int AI_enabled;
extern int LCD_enabled;
extern SemaphoreHandle_t sensorMutex;


// -----------------------------------------------------------------------------
// Global variables and synchronization objects
// -----------------------------------------------------------------------------
int update_delay = 500;                     // Delay between each serial update (in ms)
SemaphoreHandle_t serialMutex;              // Mutex for serial communication synchronization
extern QueueHandle_t irQueue;               // Queue for IR remote button data

// -----------------------------------------------------------------------------
// @brief Round a floating-point number to 2 decimal places
// @param value The float value to be rounded
// @return Rounded float with 2 decimal places
// -----------------------------------------------------------------------------
float round2(float value) {
    return round(value * 100) / 100.0;
}

// -----------------------------------------------------------------------------
// @brief Read and process JSON commands received from Raspberry Pi via Serial
// -----------------------------------------------------------------------------
void read_from_rasp()
{
    String cmd = Serial.readStringUntil('\n');   // Read command string until newline
    cmd.trim();

    StaticJsonDocument<200> raspDoc;
    DeserializationError error = deserializeJson(raspDoc, cmd);

    // Validate JSON structure
    if (error) {
        Serial.print("Invalid JSON: ");
        Serial.println(cmd);
        Serial.flush();
        return;
    }

    const char* method = raspDoc["method"];

    if (xSemaphoreTake(sensorMutex, portMAX_DELAY))
    {
        // Handle fan speed control
        if (strcmp(method, "Fan") == 0)
        {
            int percent = raspDoc["params"];
            fan_speed = map(percent, 0, 100, 0, 255);
        }
    
        // Handle AI enable/disable command
        if (strcmp(method, "AI_enabled") == 0)
        {
            AI_enabled = raspDoc["params"];
            if (AI_enabled == 0)
            {
                fan_speed = 0;
            }
        }
        xSemaphoreGive(sensorMutex);
    }

}

// -----------------------------------------------------------------------------
// @brief Task function for handling Serial communication between ESP32 and RPi
//        This function both sends sensor data and receives control commands.
// @param pvParameters FreeRTOS task parameter (unused)
// -----------------------------------------------------------------------------
void handle_serial(void *pvParameters)
{
    Serial.println("Begin handling serial commands");

    // Create mutex for synchronized serial access
    serialMutex = xSemaphoreCreateMutex();
    unsigned long lastSendTime = 0;

    while (1)
    {
        // ---------------------------------------------------------------------
        // Check for incoming serial commands
        // ---------------------------------------------------------------------
        if (Serial.available())
        {
            if (xSemaphoreTake(serialMutex, portMAX_DELAY))
            {
                read_from_rasp();                    // Parse and process JSON command
                vTaskDelay(pdMS_TO_TICKS(50));       // Short delay for stability
                xSemaphoreGive(serialMutex);
            }
        }

        // ---------------------------------------------------------------------
        // Periodically send sensor data to Raspberry Pi
        // ---------------------------------------------------------------------
        unsigned long now = millis();

        if (now - lastSendTime > update_delay)
        {
            if (xSemaphoreTake(serialMutex, portMAX_DELAY))
            {
                StaticJsonDocument<1024> espDoc;

                // Populate sensor and system data
                espDoc["brightness"] = round2(light_intensity);
                espDoc["temperature"] = round2(temperature);
                espDoc["humidity"] = round2(humidity);
                // espDoc["motion_detected"] = motion_detected;
                espDoc["remote"] = "";

                // Check if a new IR remote code has been received
                char receivedCode;
                if (xQueueReceive(irQueue, &receivedCode, 0) == pdPASS)
                {
                    char remoteStr[2] = { receivedCode, '\0' };
                    espDoc["remote"] = remoteStr;

                    // Adjust fan speed based on numeric key press (0–9)
                    if (remoteStr[0] >= '0' && remoteStr[0] <= '9')
                    {
                        int digit = remoteStr[0] - '0';
                        fan_speed = (digit * 255) / 9;
                        espDoc["Fan"] = fan_speed;
                    }
                    // Toggle LCD on/off with 'C' key
                    
                    if (remoteStr[0] == 'C')
                    {
                        LCD_enabled = !LCD_enabled;
                    }
                }

                // Serialize and send JSON to Serial
                char buffer[256];
                serializeJson(espDoc, buffer, sizeof(buffer));
                Serial.println(buffer);
                Serial.flush();
                lastSendTime = now;

                xSemaphoreGive(serialMutex);
            }
        }

        // ---------------------------------------------------------------------
        // Small delay to prevent task overload
        // ---------------------------------------------------------------------
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#endif
