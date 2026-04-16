idf_component_register(SRCS "buzzer_manager.c" "hmi_manager.c" "state_machine.c" "sw420_manager.c" "piezo_manager.c" "main.c"
                    PRIV_REQUIRES esp_adc esp_driver_ledc esp_driver_gpio esp_timer esp_driver_ledc
                    INCLUDE_DIRS ".")
