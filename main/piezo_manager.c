#include "piezo_manager.h"

#include <stdio.h>
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

#include "constants.h"
#include "state_machine.h"

#define ADC_PIN       ADC_CHANNEL_6     // Channel 6 - Check ESP32 Pinout for the GPIO Number // TODO: Make configurable
#define ADC_UNIT      ADC_UNIT_1        // ADC1
#define ADC_BITWIDTH  ADC_BITWIDTH_12   // 12-bit resolution (0-4095)
#define ADC_ATTEN     ADC_ATTEN_DB_12   // ~3.3V full-scale voltage

#define ENVELOPE_WINDOW_SIZE 8          // TODO: Make configurable
#define PIEZO_SAMPLE_FREQ 100 // Hz     // TODO: Make configurable, also increase tick rate

#define TAG "PIEZO_MANAGER"

static adc_oneshot_unit_handle_t adc_handle;
static int piezo_envelope = 0;
static event_level_t s_sw_420_event_level = EVENT_NONE;
int envelope_window_sizes[SENSITIVITY_LEVELS] = {16, 12, 8, 4, 2}; // Larger window size = more smoothing, less sensitivity. TODO: Make configurable
int envelope_threshold[SENSITIVITY_LEVELS] = {200, 150, 100, 50, 25}; // Threshold for bite detection. TODO: Make configurable

static QueueHandle_t s_event_queue = NULL;

void setup_piezo_manager(QueueHandle_t sensor_event_queue)
{
    s_event_queue = sensor_event_queue;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_PIN, &config));
}

static void update_envelope(int adc_sample) {
    int envelope_window_size = envelope_window_sizes[(int)config_manager_get_sensitivity()];
    piezo_envelope = (piezo_envelope * (envelope_window_size - 1) + adc_sample) / envelope_window_size;
}

static void vPiezoTask(void *pvParameters)
{
    int piezo_sample = 0;
    TickType_t last_wake_time = xTaskGetTickCount();
   
    while (1) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_PIN, &piezo_sample));
        update_envelope(piezo_sample);

        uint8_t threshold = envelope_threshold[config_manager_get_sensitivity()]; // TODO: Optimize so that config manager does not have to be polled every execution

        if (piezo_envelope >= threshold + 30) {
            if (s_sw_420_event_level != EVENT_HIGH) {
                sensor_event_type_t evt = SENSOR_EVENT_PIEZO_TRIGGER_HIGH;
                xQueueSend(s_event_queue, &evt, 0);
                s_sw_420_event_level = EVENT_HIGH;
            }
        }
        else if (piezo_envelope >= threshold) {
            if (s_sw_420_event_level == EVENT_NONE) {
                sensor_event_type_t evt = SENSOR_EVENT_PIEZO_TRIGGER;
                xQueueSend(s_event_queue, &evt, 0);
                s_sw_420_event_level = EVENT_LOW;
            }
        }
        else if (piezo_envelope < threshold / 2) {
            s_sw_420_event_level = EVENT_NONE;
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MS / PIEZO_SAMPLE_FREQ));
    }
}

void start_piezo_task(void) {
    xTaskCreate(vPiezoTask, "piezo", 4096, NULL, 1, NULL);
}