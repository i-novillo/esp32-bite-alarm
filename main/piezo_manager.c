#include "piezo_manager.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

#include "constants.h"

#define ADC_PIN       ADC_CHANNEL_6     // Channel 6 - Check ESP32 Pinout for the GPIO Number // TODO: Make configurable
#define ADC_UNIT      ADC_UNIT_1        // ADC1
#define ADC_BITWIDTH  ADC_BITWIDTH_12   // 12-bit resolution (0-4095)
#define ADC_ATTEN     ADC_ATTEN_DB_12   // ~3.3V full-scale voltage

#define ENVELOPE_WINDOW_SIZE 8          // TODO: Make configurable
#define PIEZO_SAMPLE_FREQ 100 // Hz     // TODO: Make configurable, also increase tick rate

#define TAG "PIEZO_MANAGER"

static adc_oneshot_unit_handle_t adc_handle;
static int piezo_envelope = 0;

int envelope_window_sizes[SENSITIVITY_LEVELS] = {16, 12, 8, 4, 2}; // Larger window size = more smoothing, less sensitivity. TODO: Make configurable
int envelope_threshold[SENSITIVITY_LEVELS] = {200, 150, 100, 50, 25}; // Threshold for bite detection. TODO: Make configurable

void setup_piezo_manager()
{
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
    int sample_count = 0;
   
    while (1) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_PIN, &piezo_sample));
        update_envelope(piezo_sample);

        // TODO: Remove this logging and replace with event triggering when envelope exceeds threshold
        if (sample_count < 10) {
            sample_count++;
        } else {
            sample_count = 0;
            ESP_LOGI(TAG, "ADC Raw Value: %d", piezo_sample);
            ESP_LOGI(TAG, "ADC Envelope Value: %d", piezo_envelope);
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MS / PIEZO_SAMPLE_FREQ));
    }
}

void start_piezo_task() {
    xTaskCreate(vPiezoTask, "piezo", 4096, NULL, 1, NULL);
}