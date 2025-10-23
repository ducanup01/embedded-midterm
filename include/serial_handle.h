#ifndef __SERIAL_HANDLE__
#define __SERIAL_HANDLE__

#include <Arduino.h>
#include <WiFi.h>

const uint16_t TCP_PORT = 69;

WiFiServer server(TCP_PORT);
WiFiClient client;

#endif