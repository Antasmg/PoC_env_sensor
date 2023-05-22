#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <iostream>
#include "esp_event.h"
#include "esp_err.h"
#include <memory>

namespace WifiCommunication
{

class WifiHandler
{
public:
    ~WifiHandler() = default;
    WifiHandler();
    esp_err_t wifiConnectSta(const std::string &ssid, const std::string &password, bool toSave);
    void wifi_disconnect(void);
    bool isConnectedToWifi;
    
private:
    WifiHandler(const WifiHandler&) = delete;
    WifiHandler(WifiHandler&&) = delete;
    WifiHandler& operator=(const WifiHandler&) = delete;
    WifiHandler& operator=(WifiHandler&&) = delete;

};

}
#endif