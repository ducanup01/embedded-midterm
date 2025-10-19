import serial, time
import paho.mqtt.client as mqttclient
import json
import RPi.GPIO as GPIO

LED_PIN = 27
NEO_PIN = 5

BROKER_ADDRESS = "app.coreiot.io"
PORT = 1883
ACCESS_TOKEN = "lEyC0sHvuh053PQT0Kv7"

GPIO.setmode(GPIO.BCM)
GPIO.setup(LED_PIN, GPIO.OUT)
GPIO.output(LED_PIN, GPIO.LOW)
GPIO.setup(NEO_PIN, GPIO.OUT)
GPIO.output(NEO_PIN, GPIO.LOW)


def subscribed(client, userdata, mid, granted_qos):
    print("Subscribed...")


def recv_message(client, userdata, message):
    global humi
    if message.payload.decode("utf-8") == "{\"method\":\"setStateLED\",\"params\":\"OFF\"}":
        GPIO.output(LED_PIN, GPIO.LOW)
        print("Turned off LED")
    elif message.payload.decode("utf-8") == "{\"method\":\"setStateLED\",\"params\":\"ON\"}":
        GPIO.output(LED_PIN, GPIO.HIGH)
        print("Turned ON LED")

def connected(client, usedata, flags, rc):
    if rc == 0:
        print("Connected successfully!!")
        client.subscribe('v1/devices/me/rpc/request/+')
    else:
        print("Connection is failed")








client = mqttclient.Client()
client.username_pw_set(ACCESS_TOKEN)

client.on_connect = connected
client.on_subscribe = subscribed
client.on_message = recv_message

client.connect(BROKER_ADDRESS, 1883)
client.loop_start()


ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
time.sleep(2)



print("Connected to: ", ser.name)

while True:
    ser.reset_input_buffer()
    line = ser.readline().decode(errors='ignore').strip()
    if line:
        try:
            data = json.loads(line)  # ESP32 already sends JSON, so parse it
            brightness = int(data.get("brightness", 0))
            if brightness % 2 == 0:
                GPIO.output(LED_PIN, GPIO.LOW)
            else:
                GPIO.output(LED_PIN, GPIO.HIGH)
            print("Sending telemetry:", data)
            # Send as JSON object, not as string
            client.publish('v1/devices/me/telemetry', json.dumps(data), qos=1)
        except json.JSONDecodeError:
            print("Invalid JSON from serial:", line)
    time.sleep(1)


### ESP CODE:

# #include <Arduino.h>

# #define LED_PIN GPIO_NUM_48
# #define LDR_PIN A3

# int light;

# void setup()
# {
#     Serial.begin(115200);
#     pinMode(LED_PIN, OUTPUT);
#     pinMode(LDR_PIN, INPUT);
#     Serial.println("Hello from ESP32!");
# }

# void loop()
# {
    
#     light = analogRead(LDR_PIN);
#     printf("\n{\"brightness\": %d}", light);

#     vTaskDelay(500);

#     light = analogRead(LDR_PIN);
#     printf("\n{\"brightness\": %d}", light);

#     vTaskDelay(500);
    
# }