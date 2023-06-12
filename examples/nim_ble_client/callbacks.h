#include <NimBLEDevice.h>

void scanEndedCB(NimBLEScanResults results) { Serial.println("Scan Ended"); }

class ClientCallbacks : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient *pClient) {
    Serial.println("Connected");
    /** After connection we should change the parameters if we don't need fast
     * response times. These settings are 150ms interval, 0 latency, 450ms
     * timout. Timeout should be a multiple of the interval, minimum is 100ms.
     *  I find a multiple of 3-5 * the interval works best for quick
     * response/reconnect. Min interval: 120 * 1.25ms = 150, Max interval: 120
     * * 1.25ms = 150, 0 latency, 60 * 10ms = 600ms timeout
     */
    pClient->updateConnParams(120, 120, 0, 60);
  };

  void onDisconnect(NimBLEClient *pClient) {
    Serial.print(pClient->getPeerAddress().toString().c_str());
    Serial.println(" Disconnected - Starting scan");
  };

  bool onConnParamsUpdateRequest(NimBLEClient *pClient,
                                 const ble_gap_upd_params *params) {
    if (params->itvl_min < 24) { /** 1.25ms units */
      return false;
    } else if (params->itvl_max > 40) { /** 1.25ms units */
      return false;
    } else if (params->latency > 2) { /** Number of intervals allowed to skip */
      return false;
    } else if (params->supervision_timeout > 100) { /** 10ms units */
      return false;
    }

    return true;
  };
};
