#ifndef SW420_MANAGER_H
#define SW420_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/**
 * @brief Sets up the SW420 functionality.
 * @param sensor_event_queue Queue to send detected SW420 events to.
 */
void setup_sw420_manager(QueueHandle_t sensor_event_queue);

/**
 * @brief Starts the SW420 task.
 */
void start_sw420_task(void);

#endif // SW420_MANAGER_H