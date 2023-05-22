#ifndef DHT22_SENSOR
#define DHT22_SENSOR

#include <iostream>
#include "esp_err.h"

namespace SensorController 
{

class SensorHandler
{

public:
    ~SensorHandler() = default;
    SensorHandler();
    bool getMeasurements();
    std::string createMessage();
    
private:
    SensorHandler(const SensorHandler&) = delete;
    SensorHandler(SensorHandler&&) = delete;
    SensorHandler& operator=(const SensorHandler&) = delete;
    SensorHandler& operator=(SensorHandler&&) = delete;
    float m_temperature;
    float m_humidity;

};

}

#endif