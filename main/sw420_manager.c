#include "sw420_manager.h"

#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "config_manager.h"
#include "constants.h"

#define SAMPLE_BUFFER_SIZE 10 // TODO: Make configurable
#define SW420_PIN GPIO_NUM_14 // TODO: Make configurable
#define SW420_SAMPLE_FREQ 10 // Hz     // TODO: Make configurable, also increase tick rate

#define TAG "SW420_MANAGER"

int vibration_threshold[SENSITIVITY_LEVELS] = {1, 2, 5, 8, 10}; // Number of vibrations detected in buffer to consider it a positive event. TODO: Make configurable
static int sw420_samples[SAMPLE_BUFFER_SIZE];


static QueueHandle_t s_event_queue = NULL;

void setup_sw420_manager(QueueHandle_t sensor_event_queue)
{
    s_event_queue = sensor_event_queue;
    
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++) {
        sw420_samples[i] = 0;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << SW420_PIN),   // Select GPIO 4
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
    TickType_t last_wake_time = xTaskGetTickCount();

    while (1) {
        sw420_samples[sample_index] = gpio_get_level(SW420_PIN);
        ESP_LOGI(TAG, "SW420 Sample: %d, index: %d", sw420_samples[sample_index], sample_index);
        sample_index = (sample_index + 1) % SAMPLE_BUFFER_SIZE;

        // if (sample_index == 9) { // TODO: Replace with event triggering when vibration count exceeds threshold
        //     ESP_LOG_BUFFER_HEX(TAG, &sw420_samples, SAMPLE_BUFFER_SIZE);
        // }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MS / SW420_SAMPLE_FREQ));
    }
}

void start_sw420_task(void)
{
    xTaskCreate(xSW420Task, "SW420 Task", 4096, NULL, 1, NULL);
}