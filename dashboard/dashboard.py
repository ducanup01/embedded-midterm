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
    try:
        payload = message.payload.decode("utf-8")
        data = json.loads(payload)
        device = data.get("method")
        action = data.get("params")

        if device == "LED":
            if action == True:
                GPIO.output(LED_PIN, GPIO.HIGH)
            elif action == False:    
                GPIO.output(LED_PIN, GPIO.LOW)

        elif device == "NEO":
            if action == True:
                GPIO.output(NEO_PIN, GPIO.HIGH)
            elif action == False:
                GPIO.output(NEO_PIN, GPIO.LOW)            

        ser.write(payload.encode('utf-8'))
        # print(payload)

    except json.JSONDecodeError:
        print("Invalid JSON received")

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
    try:
        line = ser.readline().decode(errors='ignore').strip()
        if not line:
            continue

        try:
            data = json.loads(line)
            brightness = int(data.get("brightness", 0))
            print("Sending telemetry:", data)
            client.publish('v1/devices/me/telemetry', json.dumps(data), qos=1)

        except json.JSONDecodeError:
            print("Invalid JSON from serial:", line)

        time.sleep(1)

    except serial.SerialException as e:
        print("⚠️ Serial error:", e)
        print("Reconnecting in 3s...")
        time.sleep(3)
        try:
            ser.close()
        except:
            pass
        try:
            ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
            print("✅ Reconnected to serial.")
        except:
            print("❌ Failed to reconnect.")

    
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