#include "M5StickCPlus.h"
#include <Arduino.h>
#include <ArduinoBLE.h>

extern const unsigned char ImageData[768];
extern const unsigned char icon_ble[4608];
extern const unsigned char icon_ble_disconnect[4608];

TFT_eSprite display = TFT_eSprite(&M5.Lcd);

#define SRV_UUID "19B10000-E8F2-537E-4F6C-D104768A1214" // service
#define CCC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214" // characteristic

bool state = false;

// Restart ESP32 on power button press
void checkAXPPress() {
  if (M5.Axp.GetBtnPress()) {
    do {
      delay(20);
    } while (M5.Axp.GetBtnPress());
    M5.Beep.mute();
    ESP.restart();
  }
}

void updateDisplay(BLEDevice peripheral) {
  display.fillRect(0, 0, 240, 135, TFT_BLACK);

  int cursor = 0;

  display.setCursor(12, cursor += 20);
  display.setTextSize(2);
  display.setTextColor(TFT_GREEN);
  display.printf("Central\n");
  display.setCursor(12, cursor += 25);

  display.setTextColor(TFT_RED);
  if (peripheral.connected()) {
    display.printf("Connected!\n");
  } else {
    display.printf("Scanning ...\n");
  }
  display.setCursor(12, cursor += 25);

  display.setTextColor(display.color565(18, 150, 219));
  if (peripheral.hasLocalName()) {
    display.printf(String("Name: " + peripheral.localName() + "\n").c_str());
  } else {
    display.printf("Name: NULL\n");
  }
  display.setCursor(12, cursor += 25);

  if (peripheral.hasAdvertisedServiceUuid()) {
    String uuid = peripheral.advertisedServiceUuid().substring(0, 8);
    display.printf(String("UUID: " + uuid + "\n").c_str());
  } else {
    display.printf("UUID: NULL\n");
  }

  if (peripheral.connected()) {
    display.pushImage(180, 16, 48, 48, (uint16_t *)icon_ble);
  } else {
    display.pushImage(180, 16, 48, 48, (uint16_t *)icon_ble_disconnect);
  }

  display.pushSprite(0, 0);
  M5.update();
}

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
    updateDisplay(peripheral);
    if (M5.BtnB.isPressed()) {
      Serial.println("Button B pressed, toggle LED state.");
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

    if (M5.BtnA.isPressed()) {
      Serial.println("Button A pressed, disconnect.");
      M5.Beep.tone(6000);
      delay(100);
      M5.Beep.mute();

      peripheral.disconnect();

      while (M5.BtnA.isPressed()) {
        M5.update();
        delay(10);
      }
    }

    checkAXPPress();
  }

  Serial.println("Peripheral disconnected");
}

void setup() {
  M5.begin();
  M5.Beep.mute();

  M5.Lcd.setRotation(3);
  display.createSprite(240, 135);
  display.fillRect(0, 0, 240, 135, display.color565(10, 10, 10));
  display.pushSprite(0, 0);

  Serial.begin(115200);
  BLE.begin();
  delay(100);

  Serial.println("Bluetooth® Low Energy Central - LED control");
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();
  updateDisplay(peripheral);

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
  } else {
    // peripheral disconnected, start scanning again
    Serial.println("Scanning ...");
    BLE.scanForUuid(SRV_UUID);
  }

  delay(1000);
}
