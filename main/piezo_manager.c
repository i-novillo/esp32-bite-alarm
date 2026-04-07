#include "piezo_manager.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

#define ADC_PIN       ADC_CHANNEL_6     // Channel 6 - Check ESP32 Pinout for the GPIO Number
#define ADC_UNIT      ADC_UNIT_1        // ADC1
#define ADC_BITWIDTH  ADC_BITWIDTH_12   // 12-bit resolution (0-4095)
#define ADC_ATTEN     ADC_ATTEN_DB_12   // ~3.3V full-scale voltage

#define TAG "PIEZO_MANAGER"

static adc_oneshot_unit_handle_t adc_handle;

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

static void vPiezoTask(void *pvParameters)
{
    int adc_value;
   
    while (1) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_PIN, &adc_value));

        ESP_LOGI(TAG, "ADC Raw Value: %d", adc_value);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void start_piezo_task() {
    xTaskCreate(vPiezoTask, "piezo", 4096, NULL, 1, NULL);
}