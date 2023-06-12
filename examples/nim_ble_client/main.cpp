#include <Arduino.h>
#include <M5StickCPlus.h>
#include <NimBLEDevice.h>

#include "callbacks.h"
#include "config.h"

static NimBLEAdvertisedDevice *peripheral;
static ClientCallbacks clientCB;

static bool doConnect = false;

class AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
  void onResult(NimBLEAdvertisedDevice *advertisedDevice) {
    Serial.print("Advertised Device found: ");
    Serial.println(advertisedDevice->toString().c_str());
    if (advertisedDevice->isAdvertisingService(NimBLEUUID(SRV_UUID))) {
      Serial.println("Found Our Service");

      NimBLEDevice::getScan()->stop();
      peripheral = advertisedDevice;
      doConnect = true;
    }
  };
};

void printCCCInfo(NimBLERemoteCharacteristic *pRemoteCharacteristic,
                  uint8_t *pData, size_t length, bool isNotify) {
  std::string str = (isNotify == true) ? "Notification" : "Indication";
  str += " from ";
  /** NimBLEAddress and NimBLEUUID have std::string operators */
  str += std::string(
      pRemoteCharacteristic->getRemoteService()->getClient()->getPeerAddress());
  str += ": Service = " +
         std::string(pRemoteCharacteristic->getRemoteService()->getUUID());
  str += ", Characteristic = " + std::string(pRemoteCharacteristic->getUUID());
  str += ", Value = " + std::string((char *)pData, length);
  Serial.println(str.c_str());
}

bool handleService(NimBLEClient *pClient) {
  NimBLERemoteService *pSvc = nullptr;
  NimBLERemoteCharacteristic *pChr = nullptr;
  NimBLERemoteDescriptor *pDsc = nullptr;

  pSvc = pClient->getService(SRV_UUID);
  if (!pSvc) {
    Serial.println("service not found.");
    return false;
  }

  pChr = pSvc->getCharacteristic(CCC_UUID);
  if (!pChr) {
    Serial.println("characteristic not found.");
    return false;
  }

  if (pChr->canRead()) {
    Serial.print(pChr->getUUID().toString().c_str());
    Serial.print(" Value: ");
    Serial.println(pChr->readValue().c_str());
  }

  if (pChr->canWrite()) {
    if (pChr->writeValue("Tasty")) {
      Serial.print("Wrote new value to: ");
      Serial.println(pChr->getUUID().toString().c_str());
    } else {
      pClient->disconnect();
      return false;
    }

    if (pChr->canRead()) {
      Serial.print("The value of: ");
      Serial.print(pChr->getUUID().toString().c_str());
      Serial.print(" is now: ");
      Serial.println(pChr->readValue().c_str());
    }
  }

  if (pChr->canNotify()) {
    if (!pChr->subscribe(true, printCCCInfo)) {
      pClient->disconnect();
      return false;
    }
  } else if (pChr->canIndicate()) {
    if (!pChr->subscribe(false, printCCCInfo)) {
      pClient->disconnect();
      return false;
    }
  }

  return true;
}

NimBLEClient *connect() {
  NimBLEClient *pClient = nullptr;

  /** Check if we have a client we should reuse first **/
  if (NimBLEDevice::getClientListSize()) {
    pClient = NimBLEDevice::getClientByPeerAddress(peripheral->getAddress());
    if (pClient) {
      if (!pClient->connect(peripheral, false)) {
        Serial.println("Reconnect failed");
        return nullptr;
      }
      Serial.println("Reconnected client");
    } else {
      pClient = NimBLEDevice::getDisconnectedClient();
    }
  }

  /** No client to reuse? Create a new one. */
  if (!pClient) {
    if (NimBLEDevice::getClientListSize() >= NIMBLE_MAX_CONNECTIONS) {
      Serial.println("Max clients reached - no more connections available");
      return nullptr;
    }

    pClient = NimBLEDevice::createClient();
    Serial.println("New client created");

    pClient->setClientCallbacks(&clientCB, false);
    pClient->setConnectionParams(12, 12, 0, 51);
    pClient->setConnectTimeout(5);

    if (!pClient->connect(peripheral)) {
      NimBLEDevice::deleteClient(pClient);
      Serial.println("Failed to connect, deleted client");
      return nullptr;
    }
  }

  if (!pClient->isConnected()) {
    if (!pClient->connect(peripheral)) {
      Serial.println("Failed to connect");
      return nullptr;
    }
  }

  Serial.print("Connected to: ");
  Serial.println(pClient->getPeerAddress().toString().c_str());
  Serial.print("RSSI: ");
  Serial.println(pClient->getRssi());

  return pClient;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting NimBLE Client");

  NimBLEDevice::init("");
  NimBLEDevice::setSecurityAuth(BLE_SM_PAIR_AUTHREQ_SC);

  NimBLEScan *pScan = NimBLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  pScan->setInterval(45);
  pScan->setWindow(15);
  pScan->setActiveScan(true);
  pScan->start(0, scanEndedCB); // 0 ... scan forever
}

void loop() {
  while (!doConnect) {
    delay(1);
  }

  doConnect = false;

  NimBLEClient *pClient = connect();
  if (handleService(pClient)) {
    Serial.println(
        "Success! we should now be getting notifications, scanning for more!");
  } else {
    Serial.println("Failed to connect, starting scan");
  }

  NimBLEDevice::getScan()->start(0, scanEndedCB);
}
