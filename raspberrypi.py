import asyncio
from bleak import BleakClient, BleakError
import RPi.GPIO as GPIO
import time

# Define the MAC address of the Arduino Nano 33 IoT
NANO_MAC_ADDRESS = "78:21:84:7b:3b:96"
SERVICE_UUID = "e6a4e15a-7cdd-4272-9584-84aee74d55e5"
CHARACTERISTIC_UUID = "e6a4e15a-7cdd-4272-9584-84aee74d55e5"

# Buzzer GPIO pin configuration
BUZZER_PIN = 23
GPIO.setmode(GPIO.BCM)
GPIO.setup(BUZZER_PIN, GPIO.OUT)

# Threshold for activating the buzzer
THRESHOLD_DISTANCE = 75.0  # in cm, or any unit you expect to receive

async def read_ble_data():
    async with BleakClient(NANO_MAC_ADDRESS) as client:
        print("Connected to Arduino Nano 33 IoT.")

        # Check if the device supports the service and characteristic
        try:
            while True:
                # Read data from the characteristic
                data = await client.read_gatt_char(CHARACTERISTIC_UUID)
                distance = float(data.decode('utf-8'))
                print(f"Received distance: {distance} cm")

                # Activate or deactivate buzzer based on distance
                if distance < THRESHOLD_DISTANCE:
                    GPIO.output(BUZZER_PIN, GPIO.HIGH)
                else:
                    GPIO.output(BUZZER_PIN, GPIO.LOW)

                await asyncio.sleep(1)  # Delay between reads

        except BleakError as e:
            print(f"Error: {e}")

        finally:
            # Ensure GPIO cleanup
            GPIO.cleanup()
            print("GPIO cleanup completed.")

# Run the asyncio event loop
try:
    asyncio.run(read_ble_data())
except KeyboardInterrupt:
    GPIO.cleanup()
    print("Program interrupted and GPIO cleaned up.")


