#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <iostream>
#include <memory>
#include "wifi_controller.h"
#include "MQTT_communication.h"
#include "DHT22_sensor.h"

namespace AppController 
{

class AppHandler
{
public:
    ~AppHandler() = default;
    AppHandler();
    void appLogic();
    void firstConnection(
        std::shared_ptr<WifiCommunication::WifiHandler> WiFiObject, 
        std::shared_ptr<MqttCommunication::MqttHandler> MQTTObject, 
        std::shared_ptr<SensorController::SensorHandler> DHTObject);
    void routineConnection(
        std::shared_ptr<WifiCommunication::WifiHandler> WiFiObject, 
        std::shared_ptr<MqttCommunication::MqttHandler> MQTTObject, 
        std::shared_ptr<SensorController::SensorHandler> DHTObject);
    
private:
    AppHandler(const AppHandler&) = delete;
    AppHandler(AppHandler&&) = delete;
    AppHandler& operator=(const AppHandler&) = delete;
    AppHandler& operator=(AppHandler&&) = delete;

};

}
#endif