#ifndef BLE_H
#define BLE_H
#include <iostream>
#include <memory>
#include "MQTT_communication.h"

namespace BleCommunication
{

class BleHandler
{
public:
    ~BleHandler() = default;
    static BleHandler* createInstance();
    void checkWifiStatus(esp_err_t err);
    void parseIncomingMessage(std::string& incomingMessage, const size_t size);
    BleHandler() {};
    void initBle(void);
    bool isWifiParsed;
    bool isCloudParsed;
    bool isFinishParsed;
    std::string wifiSsid;
    std::string wifiPass;
    std::string cloudAT;
    std::string cloudIP;
    uint8_t currentStatus;
    enum status {WIFI_OK, 
                 WIFI_NOT_OK, 
                 WIFI_SSID, 
                 WIFI_PASS, 
                 CLOUD_OK, 
                 CLOUD_NOTOK, 
                 FINISH_OK,
                 MEASUREMENT_NOTOK,
                 UNKNOWN};

private:
    BleHandler(const BleHandler&) = delete;
    BleHandler(BleHandler&&) = delete;
    BleHandler& operator=(const BleHandler&) = delete;
    BleHandler& operator=(BleHandler&&) = delete;

};

extern BleHandler* BLE_instance;

} // namespace BLE
#endif