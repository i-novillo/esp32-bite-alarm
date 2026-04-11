#ifndef SW420_MANAGER_H
#define SW420_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

//TODO: Add comments
void setup_sw420_manager(QueueHandle_t sensor_event_queue);

void start_sw420_task(void);

#endif // SW420_MANAGER_H