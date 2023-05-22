#ifndef MQTT_COMMUNICATION_H
#define MQTT_COMMUNICATION_H

#include <iostream>
#include "mqtt_client.h"

namespace MqttCommunication 
{

class MqttHandler
{

public:
    ~MqttHandler() = default;
    MqttHandler();
    bool mqttAppStart(const std::string& accessToken, const std::string& ipAddress);
    void publishMessage(const std::string& mqttMessage, bool toSave);
    bool disconnectMQTT();
    void checkIfConnected();
    void checkIfPublished();
    void checkIfDisconnected();
    std::string mqttAccessToken;
    std::string mqttIpAddress;
    bool isConnectedToCloud;
    bool isMessagePublished;
    bool isMqttDisconnected;
    
private:
    MqttHandler(const MqttHandler&) = delete;
    MqttHandler(MqttHandler&&) = delete;
    MqttHandler& operator=(const MqttHandler&) = delete;
    MqttHandler& operator=(MqttHandler&&) = delete;
    esp_mqtt_client_handle_t m_client = nullptr;
    esp_mqtt_client_config_t m_mqtt_cfg = {};

};

}
#endif