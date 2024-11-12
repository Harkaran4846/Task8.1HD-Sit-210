#include <ArduinoBLE.h>
#include <Wire.h>
#include <BH1750.h>

// Define custom UUIDs for the BLE service and characteristic
#define SERVICE_UUID "e6a4e15a-7cdd-4272-9584-84aee74d55e5"
#define CHARACTERISTIC_UUID "e6a4e15a-7cdd-4272-9584-84aee74d55e5"

// Initialize BLE service and characteristic
BLEService lightService(SERVICE_UUID);  // Define a new BLE service with a unique UUID
BLEStringCharacteristic lightCharacteristic(CHARACTERISTIC_UUID, BLERead | BLENotify, 20);  // Define a BLE characteristic for light data

// Initialize the BH1750 light sensor with I2C address 0x23
BH1750 lightSensor(0x23);  

// Variables for managing measurement timing
unsigned long lastMeasurementTime = 0;  // Stores the time of the last light measurement
const unsigned long measurementInterval = 1000;  // Interval for measurements in milliseconds (1 second)

void setup() {
  Serial.begin(9600);  // Initialize serial communication at 9600 baud rate
  while (!Serial);     // Wait until the serial monitor is ready

  // Initialize I2C communication and BH1750 light sensor
  Wire.begin();
  if (!lightSensor.begin()) {  // Check if the sensor initializes successfully
    Serial.println("Failed to initialize BH1750 sensor.");
    while (1);  // Halt the program if sensor initialization fails
  }

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("Failed to initialize BLE.");
    while (1);  // Halt the program if BLE initialization fails
  }

  // Set the BLE device's local name and start advertising the service
  BLE.setLocalName("Nano33IoT");
  BLE.setAdvertisedService(lightService);

  // Add the characteristic to the service
  lightService.addCharacteristic(lightCharacteristic);

  // Add the service to the BLE stack and begin advertising it
  BLE.addService(lightService);
  BLE.advertise();

  Serial.println("BLE device is active, waiting for a connection...");
}

void loop() {
  // Check for connection from a central device
  BLEDevice central = BLE.central();

  // If a central device is connected
  if (central) {
    Serial.print("Connected to central device: ");
    Serial.println(central.address());

    // While the central device is connected, read and send light sensor data
    while (central.connected()) {
      // Check if enough time has passed to take a new measurement
      if (millis() - lastMeasurementTime >= measurementInterval) {
        lastMeasurementTime = millis();  // Update the last measurement timestamp

        // Read light intensity from the sensor
        float lux = lightSensor.readLightLevel();

        // Check if the reading is valid
        if (lux >= 0) {
          Serial.print("Light Intensity: ");
          Serial.print(lux);
          Serial.println(" lux");

          // Convert lux value to a string and write it to the BLE characteristic
          String luxString = String(lux);
          lightCharacteristic.writeValue(luxString);
        } else {
          Serial.println("Failed to read light level.");  // Handle failed reading
        }
      }

      BLE.poll();  // Keep BLE connection alive and process BLE events
    }

    Serial.println("Disconnected from central device.");  // Notify disconnection
  }

  BLE.poll();  // Poll BLE events when not connected to ensure continued advertising
}
