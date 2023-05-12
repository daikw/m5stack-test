#include "M5StickCPlus.h"
#include <Arduino.h>
#include <ArduinoBLE.h>

#define SRV_UUID "19B10000-E8F2-537E-4F6C-D104768A1214" // service
#define CCC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214" // characteristic

bool state = false;

void controlLed(BLEDevice peripheral) {
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  BLECharacteristic ledCharacteristic = peripheral.characteristic(CCC_UUID);

  if (!ledCharacteristic) {
    Serial.println("Peripheral does not have LED characteristic!");
    peripheral.disconnect();
    return;
  } else if (!ledCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable LED characteristic!");
    peripheral.disconnect();
    return;
  }

  while (peripheral.connected()) {
    M5.update();
    if (M5.BtnB.isPressed()) {
      M5.Beep.tone(4000);
      delay(100);
      M5.Beep.mute();

      state = !state;
      ledCharacteristic.writeValue((byte)state);

      while (M5.BtnB.isPressed()) {
        M5.update();
        delay(10);
      }
    }
  }

  Serial.println("Peripheral disconnected");
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  BLE.begin();

  Serial.println("Bluetooth® Low Energy Central - LED control");

  Serial.println("Scanning ...");
  BLE.scanForUuid(SRV_UUID);
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised
    // service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() != "M5-LED") {
      return;
    }

    BLE.stopScan();

    controlLed(peripheral);

    // peripheral disconnected, start scanning again
    Serial.println("Scanning ...");
    BLE.scanForUuid(SRV_UUID);
  }
}
