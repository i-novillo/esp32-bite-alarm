#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "piezo_manager.h"
#include "sw420_manager.h"

void app_main(void)
{
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

    setup_sw420_manager();
    start_sw420_task();

}
