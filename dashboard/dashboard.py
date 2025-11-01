"""
Raspberry Pi MQTT & Serial Bridge for CoreIoT Platform
------------------------------------------------------
This script connects a Raspberry Pi to the CoreIoT MQTT broker and an ESP32 over serial.
It handles:
 - Receiving remote control commands from CoreIoT (via MQTT)
 - Sending commands to the ESP32 via serial
 - Reading telemetry data from ESP32
 - Publishing telemetry data to CoreIoT

Author: [Your Name]
Date: [Insert Date]
"""

import serial
import time
import paho.mqtt.client as mqttclient
import json
import RPi.GPIO as GPIO


# -------------------- GPIO Configuration --------------------
LED_PIN = 27       # Physical LED control pin
NEO_PIN = 5        # NeoPixel or secondary output pin
led_state = False  # Track LED on/off state
neo_state = False  # Track NeoPixel on/off state


# -------------------- MQTT Configuration --------------------
BROKER_ADDRESS = "app.coreiot.io"
PORT = 1883
ACCESS_TOKEN = "lEyC0sHvuh053PQT0Kv7"


# -------------------- GPIO Setup --------------------
GPIO.setmode(GPIO.BCM)
GPIO.setup(LED_PIN, GPIO.OUT)
GPIO.output(LED_PIN, GPIO.LOW)
GPIO.setup(NEO_PIN, GPIO.OUT)
GPIO.output(NEO_PIN, GPIO.LOW)


# -------------------- MQTT Callback Functions --------------------
def subscribed(client, userdata, mid, granted_qos):
    """Called when the client successfully subscribes to a topic."""
    print("✅ Subscribed to RPC topic.")


def recv_message(client, userdata, message):
    """
    Handle incoming MQTT messages from CoreIoT.
    Expects JSON payload with format:
        {"method": "<device>", "params": <True|False>}
    Controls the LED or NeoPixel pins accordingly.
    """
    try:
        payload = message.payload.decode("utf-8")
        data = json.loads(payload)
        device = data.get("method")
        action = data.get("params")

        # Handle LED control
        if device == "LED":
            if action is True:
                GPIO.output(LED_PIN, GPIO.HIGH)
            elif action is False:
                GPIO.output(LED_PIN, GPIO.LOW)

        # Handle NeoPixel control
        elif device == "NEO":
            if action is True:
                GPIO.output(NEO_PIN, GPIO.HIGH)
            elif action is False:
                GPIO.output(NEO_PIN, GPIO.LOW)

        # Forward the received command to the ESP32 via serial
        ser.write(payload.encode('utf-8'))

    except json.JSONDecodeError:
        print("⚠️ Invalid JSON received via MQTT.")


def connected(client, usedata, flags, rc):
    """
    Called when the MQTT client connects to the broker.
    Subscribes to RPC topic for receiving remote control commands.
    """
    if rc == 0:
        print("✅ Connected successfully to CoreIoT broker.")
        client.subscribe('v1/devices/me/rpc/request/+')
    else:
        print("❌ MQTT connection failed.")


# -------------------- Remote Command Handler --------------------
def handle_remote(remote_cmd):
    """
    Handle commands received from the ESP32 remote.
    Remote command options:
        A → Toggle LED
        B → Toggle NeoPixel
    """
    global led_state, neo_state

    if remote_cmd == "A":
        led_state = not led_state
        GPIO.output(LED_PIN, led_state)
        print(f"🔵 LED toggled to {led_state}")

    elif remote_cmd == "B":
        neo_state = not neo_state
        GPIO.output(NEO_PIN, neo_state)
        print(f"🟢 NEO toggled to {neo_state}")


# -------------------- MQTT Client Setup --------------------
client = mqttclient.Client()
client.username_pw_set(ACCESS_TOKEN)

client.on_connect = connected
client.on_subscribe = subscribed
client.on_message = recv_message

client.connect(BROKER_ADDRESS, PORT)
client.loop_start()


# -------------------- Serial Setup --------------------
ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
time.sleep(2)
print("🔗 Connected to serial port:", ser.name)


# -------------------- Main Loop --------------------
while True:
    try:
        # Read line from ESP32 serial output
        line = ser.readline().decode(errors='ignore').strip()
        if not line:
            continue

        try:
            # Parse incoming JSON from ESP32
            data = json.loads(line)

            # Check for remote command (A/B)
            remote_cmd = str(data.get("remote", "")).strip()
            if remote_cmd:
                handle_remote(remote_cmd)
                data.pop("remote", None)

            # Send telemetry to CoreIoT
            print("📤 To CoreIoT:", data)
            client.publish('v1/devices/me/telemetry', json.dumps(data), qos=1)

        except json.JSONDecodeError:
            print("⚠️ Invalid JSON from serial:", line)

    except serial.SerialException as e:
        # Handle serial disconnection or errors
        print("⚠️ Serial error:", e)
        print("Reconnecting in 3s...")
        time.sleep(2)
        try:
            ser.close()
        except:
            pass
        try:
            ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
            print("✅ Reconnected to serial.")
        except:
            print("❌ Failed to reconnect.")

    time.sleep(0.2)
