#include "sw420_manager.h"

#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "constants.h"
#include "hmi_manager.h"
#include "state_machine.h"

#define SAMPLE_BUFFER_SIZE 10 // TODO: Make configurable
#define SW420_PIN GPIO_NUM_14 // TODO: Make configurable
#define SW420_SAMPLE_FREQ 10 // Hz     // TODO: Make configurable, also increase tick rate

#define TAG "SW420_MANAGER"

static event_level_t s_sw_420_event_level = EVENT_NONE;
int vibration_threshold[SENSITIVITY_LEVELS] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1}; // Number of vibrations detected in buffer to consider it a positive event. TODO: Make configurable
static int sw420_samples[SAMPLE_BUFFER_SIZE];

static QueueHandle_t s_event_queue = NULL;

void setup_sw420_manager(QueueHandle_t sensor_event_queue)
{
    s_event_queue = sensor_event_queue;
    
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++) {
        sw420_samples[i] = 0;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << SW420_PIN),    // Select GPIO 14
        .mode = GPIO_MODE_INPUT,                // Set as input
        .pull_up_en = GPIO_PULLUP_ENABLE,       // Enable internal pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE,  // Disable pull-down
        .intr_type = GPIO_INTR_DISABLE          // Disable interrupts
    };
    gpio_config(&io_conf);

}

static void xSW420Task(void *pvParameters)
{
    int sample_index = 0;
    int high_count = 0;
    TickType_t last_wake_time = xTaskGetTickCount();

    while (1) {
        int old_sample = sw420_samples[sample_index];
        int new_sample = gpio_get_level(SW420_PIN);

        sw420_samples[sample_index] = new_sample;

        high_count += new_sample;
        high_count -= old_sample;

        sample_index = (sample_index + 1) % SAMPLE_BUFFER_SIZE;

        uint8_t threshold = vibration_threshold[get_sensitivity()];  // TODO: Optimize so that config manager does not have to be polled every execution

        if (high_count >= threshold + 2) {
            if (s_sw_420_event_level != EVENT_HIGH) {
                sensor_event_type_t evt = SENSOR_EVENT_SW420_TRIGGER_HIGH;
                xQueueSend(s_event_queue, &evt, 0);
                s_sw_420_event_level = EVENT_HIGH;
            }
        }
        else if (high_count >= threshold) {
            if (s_sw_420_event_level == EVENT_NONE) {
                sensor_event_type_t evt = SENSOR_EVENT_SW420_TRIGGER;
                xQueueSend(s_event_queue, &evt, 0);
                s_sw_420_event_level = EVENT_LOW;
            }
        }
        else if (high_count < threshold / 2) {
            s_sw_420_event_level = EVENT_NONE;
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MS / SW420_SAMPLE_FREQ));
    }
}

void start_sw420_task(void)
{
    xTaskCreate(xSW420Task, "SW420 Task", 4096, NULL, 3, NULL);
}