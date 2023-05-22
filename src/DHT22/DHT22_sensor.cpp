// Please keep these 2 lines at the beginning of each cpp module - tag and local log level
static const char* LOG_TAG = "DHT";
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

#include "DHT22_sensor.h"
#include "defines.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "dht.h"
#include <cmath>
#include <stdio.h>
#include <string>
#include "sleep.h"

namespace SensorController
{
#define SENSOR_TYPE DHT_TYPE_AM2301
#define DHT_PIN GPIO_NUM_33

bool SensorHandler::getMeasurements()
{
    LOG_INFO("DHT get measurement");
    esp_err_t error = dht_read_float_data(SENSOR_TYPE, DHT_PIN, &m_humidity, &m_temperature);
    if (error != ESP_OK)
    {
        LOG_ERROR("Could not read data from sensor");
        return false;
    }

    LOG_INFO("Got sensor data: humidity %.2f, Temperature %.2f", m_humidity, m_temperature);
    return true;
}

std::string SensorHandler::createMessage()
{
    std::string message = "{\"temperature\": \"";
    message += std::to_string(m_temperature);
    message += "\", \"humidity\": \"";
    message += std::to_string(m_humidity);
    message += "\"}";
    return message;
}

SensorHandler::SensorHandler()
{
    LOG_INFO("Initializing DHT22 Sensor");
}
}
