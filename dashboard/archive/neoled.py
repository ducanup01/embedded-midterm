import RPi.GPIO as GPIO
import time

# Use BCM numbering (GPIO numbers, not pin numbers)
GPIO.setmode(GPIO.BCM)

# Define the pins
pins = [5, 27]

# Set pins as output
for pin in pins:
    GPIO.setup(pin, GPIO.OUT)
    GPIO.output(pin, GPIO.LOW)

print("Toggling GPIO 5 and 27. Press Ctrl+C to stop.")

try:
    while True:
        for pin in pins:
            GPIO.output(pin, GPIO.HIGH)  # toggle
except KeyboardInterrupt:
    print("\nStopping...")
finally:
    GPIO.cleanup()  # reset pins to safe state
