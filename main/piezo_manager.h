#ifndef PIEZO_MANAGER_H
#define PIEZO_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "hmi_manager.h"

/**
 * @brief Sets up the piezo functionality (ADC startup).
 * @param sensor_event_queue Queue to send detected piezo events to.
 */
void setup_piezo_manager(QueueHandle_t sensor_event_queue);

/**
 * @brief Starts the piezo task.
 */
void start_piezo_task(void);

#endif // PIEZO_MANAGER_H