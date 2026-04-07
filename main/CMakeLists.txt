idf_component_register(SRCS "piezo_manager.c" "main.c"
                    PRIV_REQUIRES esp_adc esp_driver_ledc esp_driver_gpio
                    INCLUDE_DIRS ".")
