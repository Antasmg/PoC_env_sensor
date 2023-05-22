// Please keep these 2 lines at the beginning of each cpp module - tag and local log level
static const char* LOG_TAG = "App Controller";
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include "defines.h"
#include "sleep.h"
#include "app_controller.h"
#include "BLE.h"
#include "NVS_memory.h"
#include "nvs_flash.h"
#include "esp_sleep.h"
#include "esp_system.h"

#define SLEEP_TIME 250
#define REPEATS 5
#define DEEP_SLEEP_TIME 60000000 /* This is one minute in us */

namespace AppController
{
void AppHandler::appLogic()
{
    ConfigNvs::NvsHandler::createInstance(); /* this one needs to be created first */
    std::shared_ptr<WifiCommunication::WifiHandler> WiFiObject = std::make_shared<WifiCommunication::WifiHandler>();
    std::shared_ptr<MqttCommunication::MqttHandler> MQTTObject = std::make_shared<MqttCommunication::MqttHandler>();
    std::shared_ptr<SensorController::SensorHandler> DHTObject = std::make_shared<SensorController::SensorHandler>();

    if(!ConfigNvs::NVS_instance->isSsidConfigured || !ConfigNvs::NVS_instance->isAccessTokenConfigured)
    {
        firstConnection(WiFiObject, MQTTObject, DHTObject);
    }
    else
    {
       routineConnection(WiFiObject, MQTTObject, DHTObject);
    }
}

void AppHandler::firstConnection(
    std::shared_ptr<WifiCommunication::WifiHandler> WiFiObject, 
    std::shared_ptr<MqttCommunication::MqttHandler> MQTTObject, 
    std::shared_ptr<SensorController::SensorHandler> DHTObject)
{
    BleCommunication::BleHandler::createInstance();
    BleCommunication::BLE_instance->initBle();
    
    while (!WiFiObject->isConnectedToWifi)
    {
        while(!BleCommunication::BLE_instance->isWifiParsed)
        {
            SLEEP_MS(SLEEP_TIME);
        }
        esp_err_t connectionResult = WiFiObject->wifiConnectSta(BleCommunication::BLE_instance->wifiSsid, BleCommunication::BLE_instance->wifiPass, 1);
        BleCommunication::BLE_instance->checkWifiStatus(connectionResult);
        if (connectionResult != ESP_OK)
        {
            BleCommunication::BLE_instance->isWifiParsed = false;
        }
    }

    while (!MQTTObject->isConnectedToCloud)
    {
        while(!BleCommunication::BLE_instance->isCloudParsed)
        {
            SLEEP_MS(SLEEP_TIME);
        }

        int measureRepeats = 0;
        bool isMeasured = DHTObject->getMeasurements();
        while ((!isMeasured) && (measureRepeats <= REPEATS))
        {
            measureRepeats += 1;
            LOG_WARNING("Reps: %d", measureRepeats);
            LOG_ERROR("Failed to measure!");
            isMeasured = DHTObject->getMeasurements();
            SLEEP_MS(SLEEP_TIME);
        }

        if(isMeasured)
        {
            if (MQTTObject->mqttAppStart(BleCommunication::BLE_instance->cloudAT, BleCommunication::BLE_instance->cloudIP))
            {
                std::string mqttMessage = DHTObject->createMessage();
                MQTTObject->publishMessage(mqttMessage, 1);
                BleCommunication::BLE_instance->currentStatus = BleCommunication::BLE_instance->CLOUD_OK;
            }
            else
            {
                BleCommunication::BLE_instance->currentStatus = BleCommunication::BLE_instance->CLOUD_NOTOK;
                BleCommunication::BLE_instance->isCloudParsed = false;
            }
        }
        else
        {
            BleCommunication::BLE_instance->currentStatus = BleCommunication::BLE_instance->MEASUREMENT_NOTOK;
        }
    }

    while (!BleCommunication::BLE_instance->isFinishParsed)
    {
        SLEEP_MS(SLEEP_TIME);
    }
    if(MQTTObject->disconnectMQTT())
    {
        WiFiObject->wifi_disconnect();
    }
    BleCommunication::BLE_instance->currentStatus = BleCommunication::BLE_instance->FINISH_OK;

    LOG_WARNING("Restarting ESP");
    esp_restart();
}

void AppHandler::routineConnection(
    std::shared_ptr<WifiCommunication::WifiHandler> WiFiObject, 
    std::shared_ptr<MqttCommunication::MqttHandler> MQTTObject, 
    std::shared_ptr<SensorController::SensorHandler> DHTObject)
{
    LOG_INFO("About to connect to network after restart");
    int reconnectionAttempts = 0;
    esp_err_t err = WiFiObject->wifiConnectSta(ConfigNvs::NVS_instance->m_nvsWifiSsid, ConfigNvs::NVS_instance->m_nvsWifiPassword, 0);
    while ((reconnectionAttempts <= REPEATS) && (err != ESP_OK))
    {
        err = WiFiObject->wifiConnectSta(ConfigNvs::NVS_instance->m_nvsWifiSsid, ConfigNvs::NVS_instance->m_nvsWifiPassword, 0);
        reconnectionAttempts += 1;
        SLEEP_MS(SLEEP_TIME);
    }

    if (WiFiObject->isConnectedToWifi)
    {
        bool isMeasured = DHTObject->getMeasurements();
        if(isMeasured)
        {
            if (MQTTObject->mqttAppStart(ConfigNvs::NVS_instance->m_mqttAccessToken, ConfigNvs::NVS_instance->m_ipAddress))
            {
                std::string mqttMessage = DHTObject->createMessage();
                MQTTObject->publishMessage(mqttMessage, 0); 
            }
        }
        else
        {
            LOG_ERROR("Failed to measure!");
        }
    }
    else
    {
        LOG_ERROR("Connection failed!");
    }
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_TIME);
    LOG_WARNING("Going to deep sleep...");
    esp_deep_sleep_start();
}

AppHandler::AppHandler()
{
    LOG_INFO("App Handler");
}
}