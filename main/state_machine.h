#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

//TODO: Add comments

typedef enum {
    SENSOR_EVENT_PIEZO_TRIGGER = 0,
    SENSOR_EVENT_PIEZO_TRIGGER_HIGH = 1,
    SENSOR_EVENT_SW420_TRIGGER = 2,
    SENSOR_EVENT_SW420_TRIGGER_HIGH = 3
} sensor_event_type_t;

void setup_state_machine(QueueHandle_t sensor_event_queue);

void start_state_machine_task(void);

#endif // STATE_MACHINE_H