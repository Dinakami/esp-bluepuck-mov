set(include_dirs "include")

set(
    srcs 
        "main.c"
        "include/lightyear_bluetooth/lightyear_bluetooth_client.c"
        "bluepuck_mov/bluepuck_mov.c"
    )


set(reqs "driver esp-tls esp32 spi_flash mqtt bt spiffs wear_levelling app_update esp_https_ota json")

idf_component_register(
                    SRCS         "${srcs}"
                    REQUIRES     "${reqs}"
                    INCLUDE_DIRS "${include_dirs}"
                    )

message(STATUS "component lib: '${COMPONENT_LIB}'")
