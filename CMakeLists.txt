file(GLOB_RECURSE SOURCES
    "zenoh-pico/src/api/*.c"
    "zenoh-pico/src/collections/*.c"
    "zenoh-pico/src/link/*.c"
    "zenoh-pico/src/net/*.c"
    "zenoh-pico/src/protocol/*.c"
    "zenoh-pico/src/session/*.c"
    "zenoh-pico/src/transport/*.c"
    "zenoh-pico/src/utils/*.c"
    "zenoh-pico/src/runtime/*.c"
    "zenoh-pico/src/system/common/*.c"
    "zenoh-pico/src/system/espidf/*.c"
    "zenoh-pico/src/system/socket/esp32.c"
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "zenoh-pico/include" "zenoh-pico/include/zenoh-pico"
    REQUIRES lwip esp_timer esp_wifi esp_event freertos pthread driver
)

# Keep ONLY this block to prevent warnings
target_compile_definitions(${COMPONENT_LIB} PUBLIC 
    ZENOH_ESPIDF=1
    ZP_PLATFORM_SOCKET_ESP32=1
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-implicit-function-declaration -Wno-incompatible-pointer-types -Wno-unused-variable -Wno-error)
