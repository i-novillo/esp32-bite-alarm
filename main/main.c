#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "piezo_manager.h"

void app_main(void)
{
    // gpio_config_t io_conf = {
    //     .pin_bit_mask = (1ULL << BUTTON_PIN),   // Select GPIO 4
    //     .mode = GPIO_MODE_INPUT,                // Set as input
    //     .pull_up_en = GPIO_PULLUP_ENABLE,       // Enable internal pull-up
    //     .pull_down_en = GPIO_PULLDOWN_DISABLE,  // Disable pull-down
    //     .intr_type = GPIO_INTR_DISABLE          // Disable interrupts
    // };
    // gpio_config(&io_conf);

    // while (1) {

    //     // int level = gpio_get_level(BUTTON_PIN);
    //     // if (level == 1) {
    //     // } else {
    //     // }


    //     ESP_LOGI("ADC Value", "ADC Raw: %d", adc_value);
    //     vTaskDelay(pdMS_TO_TICKS(20));
    // }

    setup_piezo_manager();
    start_piezo_task();

}
