#ifndef PIEZO_MANAGER_H
#define PIEZO_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "config_manager.h"

/**
 * @brief Sets up the piezo functionality (ADC startup).
 */
void setup_piezo_manager(QueueHandle_t queue);

/**
 * @brief Starts the piezo task.
 */
void start_piezo_task(void);

#endif // PIEZO_MANAGER_H