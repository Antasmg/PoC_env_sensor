// Please keep these 2 lines at the beginning of each cpp module - tag and local log level
static const char* LOG_TAG = "BLE";
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include "defines.h"
#include "BLE.h"
#include <stdio.h>
#include <iostream>
#include "nvs_flash.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "esp_system.h"
#include "NVS_memory.h"
#include <esp_wifi.h>

#define DEVICE_NAME "POC ENV SENSOR"
#define MANUFACTURER_NAME 0x2A29 /* characteristic value  */

namespace BleCommunication{
void ble_app_advertise(void);
static int device_write(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt *ctxt, void *arg);
const std::string WIFI_KEY = "wifi";
const std::string CLOUD_KEY = "cloud";
const std::string FINISH_KEY = "finish";
const int ADVERTISEMENT_DURATION = 90000; /* Advertisement time is ms */ 
uint8_t ble_addr_type;
BleHandler* BLE_instance;
const ble_uuid128_t gatt_svr_svc_uart_uuid =
    BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e);
const ble_uuid128_t gatt_svr_chr_uart_write_uuid =
    BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e);
constexpr ble_gatt_svc_def gatt_svcs[] = {
    { /* This is an instance of a service */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_uart_uuid.u,
        .includes = {},
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = &gatt_svr_chr_uart_write_uuid.u,
                .access_cb = device_write,
                .arg         = {},
                .descriptors = {},
                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ,
                .min_key_size = {},
                .val_handle = {},
            },
            {
                
            }
        },
    },
    {
        /* This indicates that there are no more services */
    }
};
void BleHandler::checkWifiStatus(esp_err_t err)
{
    if (err == ESP_ERR_WIFI_SSID)
    {
        currentStatus = WIFI_SSID;
    }
    else if (err == ESP_ERR_WIFI_PASSWORD)
    {
        currentStatus = WIFI_PASS;
    }
    else if (err == ESP_FAIL)
    {
        currentStatus = WIFI_NOT_OK;
    }
    else
    {
        currentStatus = WIFI_OK;
    }
    
}
void BleHandler::parseIncomingMessage(std::string& incomingMessage, const size_t size)
{
    int index = incomingMessage.find("\n"); /* index of the first occurrence of "\n" */
    std::string command = incomingMessage.substr(0, index);
    incomingMessage = incomingMessage.substr(index+1, size);
    
    if (command == WIFI_KEY)
    {
        LOG_INFO("Command 'wifi' received");

        index = incomingMessage.find("\n");
        wifiSsid = incomingMessage.substr(0, index);
        LOG_INFO("SSID: %s", wifiSsid.c_str());

        incomingMessage = incomingMessage.substr(index+1, size);
        index = incomingMessage.find("\n");
        wifiPass = incomingMessage.substr(0, index);
        LOG_INFO("Password: %s", wifiPass.c_str());

        isWifiParsed = true; 
    }
    else if (command == CLOUD_KEY)
    {
        LOG_INFO("Command 'cloud' received");
        index = incomingMessage.find("\n");
        cloudAT = incomingMessage.substr(0, index);
        LOG_INFO("Access Token: %s", cloudAT.c_str());

        incomingMessage = incomingMessage.substr(index+1, size);
        index = incomingMessage.find("\n");
        cloudIP = incomingMessage.substr(0, index);
        LOG_INFO("IP Address: %s", cloudIP.c_str());

        isCloudParsed = true;

    }
    else if (command == FINISH_KEY)
    {
        isFinishParsed = true;
        LOG_INFO("Command 'finish' received. About to terminate connection");
        nimble_port_stop();
        nimble_port_deinit();
    }
    else
    {
        currentStatus = UNKNOWN;
        LOG_ERROR("Unknown command received");
    }
}

static int device_write(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt, void* arg)
{
    uint8_t* mess = ctxt->om->om_data; /* the message */
    uint16_t messLength = ctxt->om->om_len; /* length of the message */
    std::string incomingMessage(reinterpret_cast<char *>(mess), messLength);

    if(ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
    {
        LOG_WARNING("Incoming message: %.*s", messLength, mess);
        BLE_instance->parseIncomingMessage(incomingMessage, messLength);
    }

    switch (BLE_instance->currentStatus)
    {
    case BLE_instance->WIFI_OK:
        os_mbuf_append(ctxt->om, "Wifi\nOK", strlen("Wifi\nOK"));
        break;
    case BLE_instance->WIFI_NOT_OK:
        os_mbuf_append(ctxt->om, "Wifi\nNOT_OK", strlen("Wifi\nNOT_OK"));
        break;
    case BLE_instance->WIFI_SSID:
        os_mbuf_append(ctxt->om, "Wifi\nSSID_NOT_OK", strlen("Wifi\nSSID_NOT_OK"));
        break;
    case BLE_instance->WIFI_PASS:
        os_mbuf_append(ctxt->om, "Wifi\nPASS_NOT_OK", strlen("Wifi\nPASS_NOT_OK"));
        break;
    case BLE_instance->CLOUD_OK:
        os_mbuf_append(ctxt->om, "cloud\nOK", strlen("cloud\nOK"));
        break;
    case BLE_instance->CLOUD_NOTOK:
        os_mbuf_append(ctxt->om, "cloud\nNOT_OK", strlen("cloud\nNOT_OK"));
        break;
    case BLE_instance->FINISH_OK:
        os_mbuf_append(ctxt->om, "finish\nOK\n", strlen("finish\nOK\n"));
        break;
    case BLE_instance->MEASUREMENT_NOTOK:
        os_mbuf_append(ctxt->om, "Sensor\nNOT_OK", strlen("Sensor\nNOT_OK"));
        break;
    case BLE_instance->UNKNOWN:
        os_mbuf_append(ctxt->om, "UNKNOWN_COMMAND", strlen("UNKNOWN_COMMAND"));
        break;
    
    default:
        break;
    }

    return 0;
}

static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        LOG_INFO("BLE_GAP_EVENT_CONNECT %s", event->connect.status == 0 ? "OK" : "Failed");
        if (event->connect.status != 0)
        {
            ble_app_advertise();
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        LOG_INFO("BLE_GAP_EVENT_DISCONNECT");
        ble_app_advertise();
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        LOG_INFO("BLE_GAP_EVENT_ADV_COMPLETE");
        LOG_WARNING("Advertising time is up!");
        break;
    case BLE_GAP_EVENT_SUBSCRIBE:
        LOG_INFO("BLE_GAP_EVENT_SUBSCRIBE");
        break;
    default:
        break;
    }
    return 0;
}

void ble_app_advertise(void)
{
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_DISC_LTD;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    fields.name = (uint8_t *)ble_svc_gap_device_name();
    fields.name_len = strlen(ble_svc_gap_device_name());
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    LOG_WARNING("Advertising...");
    ble_gap_adv_start(ble_addr_type, NULL, ADVERTISEMENT_DURATION, &adv_params, ble_gap_event, NULL);
}

void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);
    ble_app_advertise();
}

void host_task(void *param)
{
    nimble_port_run();
}

void BleHandler::initBle(void)
{
    LOG_INFO("Initializing BLE");
    isWifiParsed = false;
    isCloudParsed = false;
    isFinishParsed = false;
    currentStatus = UNKNOWN;
    esp_nimble_hci_and_controller_init();
    nimble_port_init();

    ble_svc_gap_device_name_set(DEVICE_NAME);
    ble_svc_gap_init();
    ble_svc_gatt_init();

    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);

    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);
}

BleHandler* BleHandler::createInstance()
{
    if (BLE_instance == nullptr)
        BLE_instance = new BleHandler; /* new object created */
    return BLE_instance;
}

}