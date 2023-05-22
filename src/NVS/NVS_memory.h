#ifndef NVS_MEMORY_H
#define NVS_MEMORY_H

#include <iostream>

namespace ConfigNvs
{

class NvsHandler
{
public:
    ~NvsHandler() = default;
    static NvsHandler* createInstance();
    NvsHandler();
    void openNVS();
    void writeWifiSsid(const std::string& SSID);
    void writeWifiPassword(const std::string& password);
    void writeAccessToken(const std::string& accessToken);
    void writeIpAddress(const std::string& ipAddress);
    void writeString(const std::string& stringToSet, const std::string& recognitionName);
    void readWifiSsid();
    void readWifiPassword();
    void readMqttAccessToken();
    void readIpAddress();
    std::string getString(size_t maxSize, const std::string& keyName, std::string& configVariable);
    std::string m_nvsWifiPassword;
    std::string m_nvsWifiSsid;
    std::string m_mqttAccessToken;
    std::string m_ipAddress;
    bool isSsidConfigured;
    bool isPasswordConfigured;
    bool isAccessTokenConfigured;
    bool isIpAddressConfigured;

private:
    NvsHandler(const NvsHandler&) = delete;
    NvsHandler(NvsHandler&&) = delete;
    NvsHandler& operator=(const NvsHandler&) = delete;
    NvsHandler& operator=(NvsHandler&&) = delete;

};

extern NvsHandler* NVS_instance;

}
#endif