#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sdkconfig.h"

#include "constants.h"
#include "piezo_manager.h"
#include "sw420_manager.h"
#include "state_machine.h"
#include "hmi_manager.h"
#include "buzzer_manager.h"
#include "ws2812.h"

void app_main(void)
{
    
    // Initialize queues and input them into managers
    QueueHandle_t sensor_event_queue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE, sizeof(sensor_event_type_t)); // TODO: Handle error when creating

    setup_piezo_manager(sensor_event_queue);
    setup_sw420_manager(sensor_event_queue);
    setup_state_machine(sensor_event_queue);
    setup_buzzer_manager();
    setup_hmi_manager();

    start_state_machine_task();
    start_sw420_task();
    start_piezo_task();
    start_buzzer_task();
    start_hmi_task();
    // ws2812_init(0, 8); // GPIO 5, 8 LEDs

    // uint8_t r = 255;
    // uint8_t g = 0;
    // uint8_t b = 0;

    // while (1) {
    //     for(int i = 0; i<8; i++) {
    //         ws2812_set_pixel(i, r, g, b); // red
    //         ws2812_show();
    //         vTaskDelay(pdMS_TO_TICKS(500));
    //         ws2812_clear();
    //     }
    //     r = 120;
    //     g = 80;
    //     b = 45;
    //     for(int i = 0; i<8; i++) {
    //         ws2812_set_pixel(i, r, g, b); // red
    //         ws2812_show();
    //         vTaskDelay(pdMS_TO_TICKS(500));
    //         ws2812_clear();
    //     }
    //     r = 200;
    //     g = 100;
    //     b = 75;
    //     for(int i = 0; i<8; i++) {
    //         ws2812_set_pixel(i, r, g, b); // red
    //         ws2812_show();
    //         vTaskDelay(pdMS_TO_TICKS(500));
    //         ws2812_clear();
    //     }
    //     r = 12;
    //     g = 12;
    //     b = 12;
    // }
}
