#ifndef __IR_RECEIVER__
#define __IR_RECEIVER__

#include <Arduino.h>
#include <IRrecv.h>
#include <IRutils.h>

#define IR_RECV_PIN GPIO_NUM_2

IRrecv irrecv(IR_RECV_PIN);
decode_results results;

void setup_IR() {
    irrecv.enableIRIn();
}

void handle_IR(void *pvParameters) {
    setup_IR();
    
    while(1) {
        if (irrecv.decode(&results)) {
            if (results.value != 0xFFFFFFFF) {  // Ignore repeat codes
                Serial.print(results.value);     // Only print the raw value
                Serial.print("\n");              // New line
            }
            irrecv.resume();
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#endif