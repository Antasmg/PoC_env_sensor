// Please keep these 2 lines at the beginning of each cpp module - tag and local log level
static const char* LOG_TAG = "MQTT";
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include "MQTT_communication.h"
#include "defines.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "NVS_memory.h"
#include "sleep.h"
#include <stdio.h>

namespace MqttCommunication
{
#define PORT 1883
#define TOPIC "v1/devices/me/telemetry"
#define MQTT_FAIL_CODE -1
#define TIMEOUT 5000

const std::string mqttProtocol = "mqtt://";
esp_mqtt_client_handle_t client = nullptr;
esp_mqtt_event_handle_t event;
bool isPublished = false;
bool isConnected = false;

void MqttHandler::publishMessage(const std::string& mqttMessage, bool toSave)
{
    LOG_INFO("Connected! Sending a message");
    esp_mqtt_client_publish(client, TOPIC, mqttMessage.c_str(), mqttMessage.length(), 1, 0);
    LOG_INFO("Checking if message sent successfully...");
    checkIfPublished();
    int timeBlock = 0;
    while (!isMessagePublished || timeBlock <= TIMEOUT)
    {
        checkIfPublished();
        timeBlock += 10;
        SLEEP_MS(10);
    }
    if(isMessagePublished)
    {
        LOG_INFO("Message sent successfully!");
        if (toSave)
        {
            LOG_INFO("About to save Access Token and IP");
            ConfigNvs::NVS_instance->writeAccessToken(mqttAccessToken);
            ConfigNvs::NVS_instance->writeIpAddress(mqttIpAddress);
        }
    }
    else
    {
        isConnectedToCloud = false;
        LOG_ERROR("Failed to publish message. Please check credentials");
    }
}

bool MqttHandler::disconnectMQTT()
{
    LOG_WARNING("About to terminate connection to cloud and WiFi");
    esp_err_t err = esp_mqtt_client_disconnect(client);
    if (err == ESP_OK)
    {
        LOG_INFO("Disconnection init success!");
    }
    else
    {
        LOG_ERROR("Disconnection init failed!");
        isMqttDisconnected = false;
    }

    int timeBlock = 0;
    while (!isMqttDisconnected || timeBlock <= TIMEOUT)
    {
        checkIfDisconnected();
        timeBlock += 10;
        SLEEP_MS(10);
    }

    if(isMqttDisconnected)
    {
        LOG_INFO("Successfully disconnected from MQTT broker");
        return true;
    }
    else
    {
        LOG_ERROR("Error while disconnecting from MQTT broker");
        return false;
    }
}

static esp_err_t mqtt_event_handler_cb(void * eventArg)
{
    event = (esp_mqtt_event_handle_t)eventArg;
    client = event->client;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            isConnected = true;
            LOG_INFO("MQTT_EVENT_CONNECTED");
            break;
        
        case MQTT_EVENT_DISCONNECTED:
            isConnected = false;
            LOG_WARNING("MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_PUBLISHED:
            isPublished = true;
            LOG_INFO("MQTT_EVENT_PUBLISHED");
            break;

        default:
            LOG_INFO("Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

void MqttHandler::checkIfConnected()
{
    isConnectedToCloud = isConnected;
}

void MqttHandler::checkIfPublished()
{
    MqttHandler::isMessagePublished = isPublished;
}

void MqttHandler::checkIfDisconnected()
{
    MqttHandler::isMqttDisconnected = !isConnected;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    mqtt_event_handler_cb(event_data);
}

bool MqttHandler::mqttAppStart(const std::string &accessToken, const std::string &ipAddress)
{
    std::string brokerURL = mqttProtocol + ipAddress;
    m_mqtt_cfg.uri = brokerURL.c_str();
    m_mqtt_cfg.port = PORT;
    m_mqtt_cfg.username = accessToken.c_str();
    m_mqtt_cfg.disable_auto_reconnect = true; /* This one is set to true to avoid looping re-connections */

    mqttAccessToken = accessToken;
    mqttIpAddress = ipAddress;

    client = esp_mqtt_client_init(&m_mqtt_cfg);
    
    esp_err_t err = esp_mqtt_set_config(client, &m_mqtt_cfg);
    err |= esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, client);
    err |= esp_mqtt_client_start(client);

    int timeBlocker = 0;
    while (!isConnectedToCloud && timeBlocker <= TIMEOUT)
    {
        checkIfConnected();
        timeBlocker += 10;
        SLEEP_MS(10);
    }

    if (err == ESP_OK && isConnectedToCloud)
    {
        LOG_INFO("MQTT initialised successfully!");
        return true;
    }
    else
    {
        LOG_ERROR("Connection failed! Please try again!");
        isConnectedToCloud = false;
        return false;
    }
}

MqttHandler::MqttHandler()
{
    LOG_INFO("Initializing MQTT");
    isConnectedToCloud = false;
    isMessagePublished = false;
    isMqttDisconnected = false;
}
}