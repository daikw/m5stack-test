#include "M5StickCPlus.h"
#include <Arduino.h>
#include <ArduinoBLE.h>

extern const unsigned char ImageData[768];
extern const unsigned char icon_ble[4608];
extern const unsigned char icon_ble_disconnect[4608];

TFT_eSprite display = TFT_eSprite(&M5.Lcd);

#define SRV_UUID "19B10000-E8F2-537E-4F6C-D104768A1214" // service
#define CCC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214" // characteristic

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read
// and writable by central
BLEService ledService(SRV_UUID);
BLEByteCharacteristic switchCharacteristic(CCC_UUID, BLERead | BLEWrite);

const int ledPin = 10;

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

void updateDisplay(BLEDevice central) {
  display.fillRect(0, 0, 240, 135, TFT_BLACK);

  int cursor = 0;

  display.setCursor(12, cursor += 20);
  display.setTextSize(2);
  display.setTextColor(TFT_MAGENTA);
  display.printf("Peripheral\n");
  display.setCursor(12, cursor += 25);

  display.setTextColor(TFT_RED);
  if (central.connected()) {
    display.printf("Connected!\n");
  } else {
    display.printf("Disconnected\n");
  }
  display.setCursor(12, cursor += 25);

  display.setTextColor(display.color565(18, 150, 219));
  display.printf("BLE address: \n");
  display.setCursor(12, cursor += 25);

  if (central.connected()) {
    display.printf(String(central.address() + "\n").c_str());
  } else {
    display.printf("NULL\n");
  }

  if (central.connected()) {
    display.pushImage(180, 16, 48, 48, (uint16_t *)icon_ble);
  } else {
    display.pushImage(180, 16, 48, 48, (uint16_t *)icon_ble_disconnect);
  }

  display.pushSprite(0, 0);
  M5.update();
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
  pinMode(ledPin, OUTPUT);
  delay(100);
  digitalWrite(ledPin, LOW);

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
  updateDisplay(central);

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      updateDisplay(central);

      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);
        } else {
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);
        }
      }

      checkAXPPress();
    }

    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }

  delay(50);
}
