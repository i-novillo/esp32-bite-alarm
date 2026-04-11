#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MS 1000
#define SENSOR_EVENT_QUEUE_SIZE 20 // TODO: Make configurable

typedef enum {
    EVENT_NONE = 0,
    EVENT_LOW = 1,
    EVENT_HIGH = 2
} event_level_t;

#endif // CONSTANTS_H