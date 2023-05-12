#include "M5StickCPlus.h"
#include <Arduino.h>
#include <ArduinoBLE.h>

#define SRV_UUID "19B10000-E8F2-537E-4F6C-D104768A1214" // service
#define CCC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214" // characteristic

BLEService ledService(SRV_UUID);

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read
// and writable by central
BLEByteCharacteristic switchCharacteristic(CCC_UUID, BLERead | BLEWrite);

const int ledPin = 10;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  pinMode(ledPin, OUTPUT);

  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1)
      ;
  }

  BLE.setLocalName("M5-LED");
  BLE.setAdvertisedService(ledService);

  ledService.addCharacteristic(switchCharacteristic);
  BLE.addService(ledService);
  switchCharacteristic.writeValue(0);

  Serial.println("start advertising");
  BLE.advertise();
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);
        } else {
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);
        }
      }
    }

    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}
