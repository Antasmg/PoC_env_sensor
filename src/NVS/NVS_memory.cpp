// Please keep these 2 lines at the beginning of each cpp module - tag and local log level
static const char* LOG_TAG = "NVS";
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include "defines.h"
#include "NVS_memory.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include <stdio.h>
#include <cstring>
#include <iostream>

#define MAX_WIFI_SSID_LENGTH 32
#define MAX_WIFI_PASSWORD_LENGTH 64
#define MAX_MQTT_ACCESS_TOKEN_LENGTH 64
#define MAX_IP_ADDRESS_LENGTH 16

namespace ConfigNvs
{
NvsHandler* NVS_instance;
const std::string NAME_WIFI_SSID = "w-ssid"; 
const std::string NAME_WIFI_PASSWORD = "w-pass";
const std::string NAME_MQTT_ACCESS_TOKEN = "m-actk";
const std::string NAME_IP_ADDRESS = "m-ipad";
nvs_handle credentialsHandle;


void NvsHandler::writeWifiSsid(const std::string& SSID)
{
    writeString(SSID, NAME_WIFI_SSID);
}

void NvsHandler::writeWifiPassword(const std::string& password)
{
    writeString(password, NAME_WIFI_PASSWORD);
}

void NvsHandler::writeAccessToken(const std::string& accessToken)
{
    writeString(accessToken, NAME_MQTT_ACCESS_TOKEN);
}

void NvsHandler::writeIpAddress(const std::string& ipAddress)
{
    writeString(ipAddress, NAME_IP_ADDRESS);
}

void NvsHandler::writeString(const std::string& stringToSet, const std::string& recognitionName)
{
    esp_err_t err = nvs_set_str(credentialsHandle, recognitionName.c_str(), stringToSet.c_str());
    if (err != ESP_OK)
    {
        LOG_INFO("Error (%s) writing to NVS!\n", esp_err_to_name(err));
        return;
    }
    nvs_commit(credentialsHandle);
    LOG_INFO("%s written to NVS", recognitionName.c_str());
}

std::string NvsHandler::getString(size_t maxSize, const std::string& keyName, std::string& configVariable)
{
    char buffer[maxSize];
    std::memset(buffer, 0, maxSize);
    esp_err_t errorCode = ::nvs_get_str(credentialsHandle, keyName.c_str(), buffer, &maxSize);
    switch (errorCode)
    {
        case ESP_OK:
        {
            size_t length = ::strlen(buffer);
            configVariable = std::string(buffer, length);
            configVariable[maxSize - 1] = 0;
            LOG_INFO("Config variable %s: %s", keyName.c_str(), configVariable.c_str());
            return configVariable;
            break;
        }
        case ESP_ERR_NVS_NOT_FOUND:
        {
            LOG_WARNING("Config variable - not initialized yet");
            configVariable.clear();
            return configVariable;
            break;
        }
        default:
        {
            LOG_ERROR("Error (%s) reading!", esp_err_to_name(errorCode));
            configVariable.clear();
            return configVariable;
            break;
        }
    }
}

void NvsHandler::readWifiSsid()
{
    std::string wifiSsid = getString(MAX_WIFI_SSID_LENGTH, NAME_WIFI_SSID, m_nvsWifiSsid);
    if (!wifiSsid.empty())
    {
        isSsidConfigured = true;
    }
    else
    {
        isSsidConfigured = false;
    }
}

void NvsHandler::readWifiPassword()
{
    std::string wifiPass = getString(MAX_WIFI_PASSWORD_LENGTH, NAME_WIFI_PASSWORD, m_nvsWifiPassword);
}

void NvsHandler::readMqttAccessToken()
{
    std::string accessToken = getString(MAX_MQTT_ACCESS_TOKEN_LENGTH, NAME_MQTT_ACCESS_TOKEN, m_mqttAccessToken);
}

void NvsHandler::readIpAddress()
{
    std::string ipAddress = getString(MAX_IP_ADDRESS_LENGTH, NAME_IP_ADDRESS, m_ipAddress);
}

void NvsHandler::openNVS()
{
    LOG_INFO("Opening Non-Volatile Storage (NVS) handle");

    esp_err_t err = nvs_open("storage", NVS_READWRITE, &credentialsHandle);
    if (err != ESP_OK)
    {
        LOG_ERROR("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }
}

NvsHandler::NvsHandler()
{
    nvs_flash_init();
    openNVS();
    LOG_ERROR("About to read from NVS");
    readWifiSsid();
    readWifiPassword();
    readIpAddress();
    readMqttAccessToken();
}

NvsHandler* NvsHandler::createInstance()
{
    if (NVS_instance == nullptr)
        NVS_instance = new NvsHandler; /* new object created */
    return NVS_instance;
}
}