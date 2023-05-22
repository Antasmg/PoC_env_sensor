// Please keep these 2 lines at the beginning of each cpp module - tag and local log level
static const char* LOG_TAG = "Main";
#define LOG_LOCAL_LEVEL ESP_LOG_INFO


#include "defines.h"
#include "sleep.h"
#include "BLE.h"
#include "NVS_memory.h"
#include "wifi_controller.h"
#include "nvs_flash.h"
#include "MQTT_communication.h"
#include "app_controller.h"

void run(void);

extern "C"
{
    void app_main(void)
    {
        run();
    }
}

void run(void)
{
    LOG_INFO("Hello from main!");
    AppController::AppHandler mainApp;
    mainApp.appLogic();
}