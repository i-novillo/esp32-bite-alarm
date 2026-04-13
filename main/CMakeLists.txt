idf_component_register(SRCS "state_machine.c" "sw420_manager.c" "config_manager.c" "piezo_manager.c" "main.c"
                    PRIV_REQUIRES esp_adc esp_driver_ledc esp_driver_gpio
                    INCLUDE_DIRS ".")
