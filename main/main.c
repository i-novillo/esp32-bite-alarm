#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sdkconfig.h"

#include "constants.h"
#include "config_manager.h"
#include "piezo_manager.h"
#include "sw420_manager.h"
#include "state_machine.h"

void app_main(void)
{
    
    // Initialize queues and input them into managers
    QueueHandle_t sensor_event_queue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE, sizeof(sensor_event_type_t));

    setup_piezo_manager(sensor_event_queue);
    setup_sw420_manager(sensor_event_queue);
    setup_state_machine(sensor_event_queue);

    start_state_machine_task();
    start_sw420_task();
    start_piezo_task();

}
