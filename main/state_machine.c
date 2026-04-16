#include "state_machine.h"
#include "buzzer_manager.h"

#include <stdio.h>
#include "esp_log.h"

#define TAG "STATE_MACHINE"

#define STATE_READY_QUEUE_TIMEOUT_MS 1000 // TODO: Make configurable and adjust through sensitivity
#define STATE_POSSIBLE_QUEUE_TIMEOUT_MS 500 // TODO: Make configurable and adjust through sensitivity
#define STATE_ALARMING_QUEUE_TIMEOUT_MS 600 // TODO: Make configurable and adjust through sensitivity

#define SCORE_DECAY 2 // TODO: Make configurable and adjust through sensitivity
#define START_TIMEOUT_SCORE_THRESHOLD 10 // TODO: Make configurable and adjust through sensitivity
#define START_ALARMING_SCORE_THRESHOLD 20 // TODO: Make configurable and adjust through sensitivity
#define END_ALARMING_SCORE_THRESHOLD 14 // TODO: Make configurable and adjust through sensitivity
#define MAX_BITE_SCORE 50 // TODO: Make configurable and adjust through sensitivity

#define TIMEOUT_DURATION_MS 1000 // TODO: Make configurable and adjust through sensitivity

/*
    SENSOR_EVENT_PIEZO_TRIGGER: 4 points
    SENSOR_EVENT_PIEZO_TRIGGER_HIGH: 8 points
    SENSOR_EVENT_SW420_TRIGGER: 2 points
    SENSOR_EVENT_SW420_TRIGGER_HIGH: 6 points
*/
int event_bite_score[4] = {4, 8, 2, 6}; // TODO: Make configurable and adjust through sensitivity

typedef enum {
    STATE_IDLE,
    STATE_READY,
    STATE_POSSIBLE_BITE,
    STATE_CONFIRMED_BITE,
    STATE_ALARMING,
    STATE_TIMEOUT
} state_t;

static state_t current_state = STATE_IDLE;
static __uint8_t bite_score = 0;
static TickType_t queue_timeout = 0;
static QueueHandle_t s_event_queue = NULL;
static bool alarm_active = false;

void setup_state_machine(QueueHandle_t sensor_event_queue)
{
    s_event_queue = sensor_event_queue;
}

static void process_sensor_event(sensor_event_type_t event)
{
    switch (current_state)
    {
        case STATE_READY:
            current_state = STATE_POSSIBLE_BITE;
            queue_timeout = pdMS_TO_TICKS(STATE_POSSIBLE_QUEUE_TIMEOUT_MS);
            bite_score = 0;
            buzzer_set_state(BUZZER_ON_POSSIBLE_BITE);
            break;

        case STATE_POSSIBLE_BITE:
            bite_score = (bite_score + event_bite_score[event]) % MAX_BITE_SCORE;
            if (bite_score >= START_ALARMING_SCORE_THRESHOLD) {
                current_state = STATE_CONFIRMED_BITE;
                queue_timeout = pdMS_TO_TICKS(0);
            }
            break;  

        case STATE_CONFIRMED_BITE:
            current_state = STATE_ALARMING;
            queue_timeout = pdMS_TO_TICKS(STATE_ALARMING_QUEUE_TIMEOUT_MS);
            alarm_active = true;
            buzzer_set_state(BUZZER_ON_ALARM);
            break;

        case STATE_ALARMING:
            bite_score = (bite_score + event_bite_score[event]) % MAX_BITE_SCORE;
            break;

        case STATE_TIMEOUT:
                vTaskDelay(pdMS_TO_TICKS(TIMEOUT_DURATION_MS));
                xQueueReset(s_event_queue);
                current_state = STATE_READY;
                queue_timeout = pdMS_TO_TICKS(STATE_READY_QUEUE_TIMEOUT_MS);
            break;

        default:
            break;
    }
}

static void process_sensor_event_timeout()
{
    switch (current_state)
    {
        case STATE_POSSIBLE_BITE:
            if (bite_score <= SCORE_DECAY) {
                current_state = STATE_READY;
                queue_timeout = pdMS_TO_TICKS(STATE_READY_QUEUE_TIMEOUT_MS);
            }
            bite_score -= SCORE_DECAY; // TODO: Make configurable and adjust through sensitivity
            if (alarm_active && bite_score < START_TIMEOUT_SCORE_THRESHOLD) {
                current_state = STATE_TIMEOUT;
                alarm_active = false;
                queue_timeout = pdMS_TO_TICKS(0);
            }
            break;  

        case STATE_CONFIRMED_BITE:
            current_state = STATE_ALARMING;
            queue_timeout = pdMS_TO_TICKS(STATE_ALARMING_QUEUE_TIMEOUT_MS);
            alarm_active = true;
            buzzer_set_state(BUZZER_ON_ALARM);
            break;

        case STATE_ALARMING:
            bite_score -= SCORE_DECAY; // TODO: Make configurable and adjust through sensitivity
            if (bite_score < END_ALARMING_SCORE_THRESHOLD) {
                current_state = STATE_POSSIBLE_BITE;
                queue_timeout = pdMS_TO_TICKS(STATE_POSSIBLE_QUEUE_TIMEOUT_MS);
                buzzer_set_state(BUZZER_OFF);
            }
            break;
            
        case STATE_TIMEOUT:
            vTaskDelay(pdMS_TO_TICKS(TIMEOUT_DURATION_MS));
            current_state = STATE_READY;
            queue_timeout = pdMS_TO_TICKS(STATE_READY_QUEUE_TIMEOUT_MS);
            xQueueReset(s_event_queue);
            break;

        default:
            break;
    }
}

static void xTaskStateMachine(void *pvParameters)
{
    current_state = STATE_READY;
    queue_timeout = pdMS_TO_TICKS(STATE_READY_QUEUE_TIMEOUT_MS);

    while (1) {
        sensor_event_type_t event;
        if (xQueueReceive(s_event_queue, &event, queue_timeout) == pdPASS) {
            ESP_LOGI(TAG, "Received sensor event: %d", event); // TODO: Make debug
            process_sensor_event(event);
        } else {
            ESP_LOGI(TAG, "Sensor event queue timeout"); // TODO: Make debug
            process_sensor_event_timeout();
        }

    }
}

void start_state_machine_task()
{
    xTaskCreate(xTaskStateMachine, "State Machine Task", 4096, NULL, 2, NULL);
}