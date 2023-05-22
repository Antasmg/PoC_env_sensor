// Please keep these 2 lines at the beginning of each cpp module - tag and local log level
static const char* LOG_TAG = "WiFi";
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include "wifi_controller.h"
#include "defines.h"
#include "esp_netif.h"
#include "esp_system.h"
#include <esp_wifi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "NVS_memory.h"
#include <stdio.h>
#include <string.h>

namespace WifiCommunication
{
#define TIMEOUT 60000 /* time in ms to wait for connection */
#define CONNECTED_GOT_IP BIT0
#define DISCONNECTED BIT1
#define SSID_NOT_OK BIT2
#define PASS_NOT_OK BIT3

esp_netif_t *espNetif;
static EventGroupHandle_t wifi_events;

void wifi_disconnect(void);

void determine_disconnect_reason(uint8_t disconnectReason)
{
    switch (disconnectReason)
    {
    case WIFI_REASON_ASSOC_LEAVE:
        LOG_ERROR("Disconnected");
        xEventGroupSetBits(wifi_events, DISCONNECTED);
        break;
    
    case WIFI_REASON_NO_AP_FOUND:
        LOG_ERROR("SSID not found");
        xEventGroupSetBits(wifi_events, SSID_NOT_OK);
        break;
    
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
        /* Four-way handshake times out. Setting wrong password when STA connecting to an encrypted AP. */
        LOG_ERROR("Wrong password!");
        xEventGroupSetBits(wifi_events, PASS_NOT_OK);
        break;
    
    default:
        xEventGroupSetBits(wifi_events, DISCONNECTED);
        break;
    }
}

void event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    switch (event_id)
    {
    case SYSTEM_EVENT_STA_START:
        LOG_INFO("Connecting...");
        esp_wifi_connect();
        break;
        
    case SYSTEM_EVENT_STA_CONNECTED:
        LOG_INFO("Connected!");
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
    {
        wifi_event_sta_disconnected_t* wifi_event_sta_disconnected = (wifi_event_sta_disconnected_t*) event_data;
        determine_disconnect_reason(wifi_event_sta_disconnected->reason);
    }
    break;

    case IP_EVENT_STA_GOT_IP:
        LOG_INFO("Got IP");
        xEventGroupSetBits(wifi_events, CONNECTED_GOT_IP);
        break;

    default:
        break;
    }
}

bool wifi_init(void)
{
    esp_err_t err = esp_netif_init();
    err |= esp_event_loop_create_default();
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    err |= esp_wifi_init(&wifi_init_config);
    err |= esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL);
    err |= esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL);
    err |= esp_wifi_set_storage(WIFI_STORAGE_RAM);

    if (err != 0)
    {
        LOG_ERROR("Initialization failed!");
        return false;
    }

    return true;
}

esp_err_t WifiHandler::wifiConnectSta(const std::string &ssid, const std::string &password, bool toSave)
{
    isConnectedToWifi = false;
    wifi_events = xEventGroupCreate();
    espNetif = esp_netif_create_default_wifi_sta(); /* creating a station */

    if(ssid == "")
    {
        LOG_ERROR("Empty SSID! Probably not initialized yet");
        wifi_disconnect();
        return ESP_ERR_WIFI_SSID;
    }

    wifi_config_t wifiConfig;
    memset(&wifiConfig, 0, sizeof(wifiConfig));
    memcpy(&wifiConfig.sta.ssid, ssid.c_str(), sizeof(wifiConfig.sta.ssid));
    memcpy(&wifiConfig.sta.password, password.c_str(), sizeof(wifiConfig.sta.password));

    esp_err_t err = esp_wifi_set_mode(WIFI_MODE_STA);
    err |= esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    err |= esp_wifi_start();
    if (err)
    {
        LOG_ERROR("STA mode init failed!");
        return ESP_FAIL;
    }

    EventBits_t result = xEventGroupWaitBits(
        wifi_events, 
        CONNECTED_GOT_IP | DISCONNECTED | SSID_NOT_OK | PASS_NOT_OK, 
        true, 
        false, 
        pdMS_TO_TICKS(TIMEOUT));

    if (result == CONNECTED_GOT_IP)
    {
        isConnectedToWifi = true;
        if (toSave)
        {
            LOG_INFO("Connected! About to save to NVS...");
            ConfigNvs::NVS_instance->writeWifiSsid(ssid);
            ConfigNvs::NVS_instance->writeWifiPassword(password);
        }
        return ESP_OK;
    }
    else if (result == SSID_NOT_OK)
    {
        LOG_ERROR("Invalid SSID");
        isConnectedToWifi = false;
        wifi_disconnect();
        return ESP_ERR_WIFI_SSID;
    }
    else if (result == PASS_NOT_OK)
    {
        LOG_ERROR("Invalid password");
        isConnectedToWifi = false;
        wifi_disconnect();
        return ESP_ERR_WIFI_PASSWORD;
    }
    isConnectedToWifi = false;
    wifi_disconnect();
    return ESP_FAIL;
}

void WifiHandler::wifi_disconnect(void)
{
    esp_err_t err = esp_wifi_disconnect();
    err |= esp_wifi_stop();
    if (err)
    {
        LOG_ERROR("Disconnection failed!");
    }
    esp_netif_destroy_default_wifi(espNetif);
}

WifiHandler::WifiHandler()
{
    LOG_INFO("ESP_WIFI_MODE_STA");
    isConnectedToWifi = false;
    if (wifi_init() == false)
    {
        LOG_ERROR("Can not initialize wifi!");
    }
}
}