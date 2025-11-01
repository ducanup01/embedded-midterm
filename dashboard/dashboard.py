import serial, time
import paho.mqtt.client as mqttclient
import json
import RPi.GPIO as GPIO

LED_PIN = 27
NEO_PIN = 5
led_state = False
neo_state = False

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



def handle_remote(remote_cmd):
    """
    Toggle LED or NEO pins based on remote command.
    A → toggle LED
    B → toggle NEO
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
            # brightness = int(data.get("brightness", 0))

            remote_cmd = str(data.get("remote", "")).strip()
            if remote_cmd:
                handle_remote(remote_cmd)
                data.pop("remote", None)

            print("To CoreIOT:", data)
            client.publish('v1/devices/me/telemetry', json.dumps(data), qos=1)

        except json.JSONDecodeError:
            print("Invalid JSON from serial:", line)

    except serial.SerialException as e:
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