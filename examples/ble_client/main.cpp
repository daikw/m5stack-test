#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>

#include "M5StickCPlus.h"

extern const unsigned char ImageData[768];
extern const unsigned char icon_ble[4608];
extern const unsigned char icon_ble_disconnect[4608];

TFT_eSprite Disbuff = TFT_eSprite(&M5.Lcd);

// convert u_int8_t to hex string format
String hexToString(uint8_t *data, int len) {
  String str = "";
  for (int i = 0; i < len; i++) {
    char tmp[3];
    sprintf(tmp, "%02X", data[i]);
    str += tmp;
  }
  return str;
}

void checkAXPPress() {
  if (M5.Axp.GetBtnPress()) {
    do {
      delay(20);
    } while (M5.Axp.GetBtnPress());
    M5.Beep.mute();
    ESP.restart();
  }
}

void Displaybuff() { Disbuff.pushSprite(0, 0); }

#define SERVICE_UUID "1bc68b2a-f3e3-11e9-81b4-2a2ae2dbcce4"
#define CHARACTERISTIC_RX_UUID "1bc68da0-f3e3-11e9-81b4-2a2ae2dbcce4"
#define CHARACTERISTIC_TX_UUID "1bc68efe-f3e3-11e9-81b4-2a2ae2dbcce4"

static BLEUUID serviceUUID(SERVICE_UUID);
static BLEUUID charUUID(CHARACTERISTIC_RX_UUID);

bool deviceConnected = false;
bool doConnect = false;
bool doScan = false;

BLERemoteCharacteristic *pRemoteCharacteristic;
BLEAdvertisedDevice *targetDevice;

BLEService *pService = NULL;
BLECharacteristic *pTxCharacteristic;

uint8_t *data = new uint8_t[128];

static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                           uint8_t *pData, size_t length, bool isNotify) {
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.println((char *)pData);
}

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(targetDevice->getAddress().toString().c_str());

  BLEClient *pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  class Callbacks : public BLEClientCallbacks {
    void onConnect(BLEClient *pclient) {}

    void onDisconnect(BLEClient *pclient) {
      deviceConnected = false;
      Serial.println("onDisconnect");
    }
  };
  pClient->setClientCallbacks(new Callbacks());

  pClient->connect(targetDevice);
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  // Read the value of the characteristic.
  if (pRemoteCharacteristic->canRead()) {
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if (pRemoteCharacteristic->canNotify()) {
    pRemoteCharacteristic->registerForNotify(notifyCallback);
  }

  deviceConnected = true;
  return true;
}

bool InitBLEClient() {
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  /**
   * Scan for BLE servers and find the first one that advertises the service we
   * are looking for.
   */
  class Callbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found: ");
      Serial.print(advertisedDevice.getServiceUUIDCount());
      Serial.println(advertisedDevice.toString().c_str());

      if (advertisedDevice.haveServiceUUID() &&
          advertisedDevice.isAdvertisingService(serviceUUID)) {

        BLEDevice::getScan()->stop();
        targetDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = true;
      }
    }
  };

  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new Callbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  return true;
}

void DisPlayBLEConnect() {
  while ((!M5.BtnA.isPressed()) && (!M5.BtnB.isPressed())) {
    Disbuff.fillRect(0, 0, 240, 135, TFT_BLACK);

    int cursor = 0;

    Disbuff.setCursor(12, cursor += 20);
    Disbuff.setTextSize(2);
    Disbuff.setTextColor(TFT_GREEN);
    Disbuff.printf("Central\n");
    Disbuff.setCursor(12, cursor += 25);

    Disbuff.setTextColor(TFT_RED);
    if (deviceConnected) {
      Disbuff.printf("BLE connected!\n");
    } else {
      Disbuff.printf("BLE disconnect\n");
    }
    Disbuff.setCursor(12, cursor += 25);

    Disbuff.setTextColor(Disbuff.color565(18, 150, 219));
    Disbuff.printf(String("Name: TODO \n").c_str());
    Disbuff.setCursor(12, cursor += 25);

    Disbuff.printf("UUID: TODO\n");

    if (deviceConnected) {
      Disbuff.pushImage(180, 16, 48, 48, (uint16_t *)icon_ble);
    } else {
      Disbuff.pushImage(180, 16, 48, 48, (uint16_t *)icon_ble_disconnect);
    }

    Displaybuff();

    M5.update();
    delay(100);
    checkAXPPress();
  }

  while ((M5.BtnA.isPressed()) || (M5.BtnB.isPressed())) {
    M5.update();
    checkAXPPress();
    M5.Beep.tone(4000);
    delay(10);
  }

  delay(50);
  M5.Beep.mute();
  Disbuff.setTextColor(TFT_WHITE);
}

void setup() {
  M5.begin();
  Wire.begin(32, 33);
  M5.Lcd.setRotation(3);

  Disbuff.createSprite(240, 135);
  Disbuff.fillRect(0, 0, 240, 135, Disbuff.color565(10, 10, 10));
  Disbuff.pushSprite(0, 0);
  delay(500);

  M5.update();
  if (M5.BtnB.isPressed()) {
    M5.Beep.tone(4000);
    delay(100);
    M5.Beep.mute();

    while (M5.BtnB.isPressed()) {
      M5.update();
      delay(10);
    }
  }
  M5.Axp.ScreenBreath(12);

  InitBLEClient();

  Disbuff.pushSprite(0, 0);
}

void loop() {
  DisPlayBLEConnect();

  if (doConnect) {
    if (connectToServer()) {
      Serial.println("connected");
    } else {
      Serial.println("failed");
    }
    doConnect = false;
  }

  if (deviceConnected) {
    String newValue = "Time since boot: " + String(millis() / 1000);
    Serial.println("Setting new characteristic value to \"" + newValue + "\"");

    // pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
  } else if (doScan) {
    BLEDevice::getScan()->start(0);
  }

  M5.update();
  delay(50);
}
