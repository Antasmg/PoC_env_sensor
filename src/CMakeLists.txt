cmake_minimum_required(VERSION 3.10)


set (SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)


set (ALL_SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/app_controller.cpp
    ${SRC_DIR}/app_controller.h
    ${SRC_DIR}/BLE/BLE.cpp
    ${SRC_DIR}/BLE/BLE.h
    ${SRC_DIR}/NVS/NVS_memory.cpp
    ${SRC_DIR}/NVS/NVS_memory.h
    ${SRC_DIR}/WiFi/wifi_controller.cpp
    ${SRC_DIR}/WiFi/wifi_controller.h
    ${SRC_DIR}/MQTT/MQTT_communication.cpp
    ${SRC_DIR}/MQTT/MQTT_communication.h
    ${SRC_DIR}/DHT22/DHT22_sensor.cpp
    ${SRC_DIR}/DHT22/DHT22_sensor.h
    ${SRC_DIR}/common/defines.h
    ${SRC_DIR}/common/sleep.h
    ${SRC_DIR}/common/utils.c
    ${SRC_DIR}/common/utils.h
    ${SRC_DIR}/common/error_codes.c
    ${SRC_DIR}/common/error_codes.h
    )


set (ALL_INCLUDE_DIRS
    ${SRC_DIR}/
    ${SRC_DIR}/common/
    ${SRC_DIR}/MQTT/
    ${SRC_DIR}/BLE/
    ${SRC_DIR}/NVS/
    ${SRC_DIR}/WiFi/
    ${SRC_DIR}/DHT22/
    )


set(COMPONENT_REQUIRES 
    nvs_flash
    log
    freertos
    driver
    lwip
    bt
    esp_eth
    mqtt
    dht
    )

set(COMPONENT_PRIV_REQUIRES)

set(COMPONENT_SRCS ${ALL_SOURCES})
set(COMPONENT_ADD_INCLUDEDIRS ${ALL_INCLUDE_DIRS})


register_component()

#if (${BUILD_WITH_PRINTS_AND_LOGS})
#    message(WARNING "!!! Building program with prints and logs!")
#    add_definitions(-DENABLE_PRINTS_AND_LOGS)
#endif()


#target_compile_options(${COMPONENT_TARGET} PRIVATE -std=gnu99)  // already set by idf.py
target_compile_options(${COMPONENT_TARGET} PRIVATE -Wall)
