#include "ws2812.h"
#include "driver/rmt_tx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>

#define TAG "WS2812"

// RMT handles
static rmt_channel_handle_t channel = NULL;
static rmt_encoder_handle_t encoder = NULL;

// LED buffer
static uint8_t *led_buffer = NULL;
static int led_count = 0;

// ------------------------
// Init
// ------------------------

void ws2812_init(int gpio, int count)
{
    led_count = count;
    led_buffer = calloc(count * 3, sizeof(uint8_t));
    if (!led_buffer) {
        ESP_LOGE(TAG, "Failed to allocate LED buffer");
        return;
    }

    // C3 prefers REF clock (more stable timing)
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = gpio,
        .mem_block_symbols = 64,
        .resolution_hz = 10 * 1000 * 1000, // 10 MHz (0.1us per tick)
        .trans_queue_depth = 4,
    };

    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &channel));

    // WS2812 timing (slightly conservative for stability)
    rmt_bytes_encoder_config_t encoder_config = {
        .bit0 = {
            .level0 = 1,
            .duration0 = 3,   // 0.3us
            .level1 = 0,
            .duration1 = 9,   // 0.9us
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = 9,   // 0.9us
            .level1 = 0,
            .duration1 = 3,   // 0.3us
        },
        .flags.msb_first = 1
    };

    ESP_ERROR_CHECK(rmt_new_bytes_encoder(&encoder_config, &encoder));

    ESP_ERROR_CHECK(rmt_enable(channel));
}

// ------------------------
// Set pixel
// ------------------------

void ws2812_set_pixel(int index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index >= led_count) return;

    // WS2812 uses GRB order
    led_buffer[index * 3 + 0] = g;
    led_buffer[index * 3 + 1] = r;
    led_buffer[index * 3 + 2] = b;
}

// ------------------------

void ws2812_clear(void)
{
    memset(led_buffer, 0, led_count * 3);
}

// ------------------------

void ws2812_show(void)
{
    rmt_transmit_config_t tx_config = {
        .loop_count = 0,
    };

    ESP_ERROR_CHECK(rmt_transmit(channel, encoder,
                                 led_buffer, led_count * 3,
                                 &tx_config));

    ESP_ERROR_CHECK(rmt_tx_wait_all_done(channel, portMAX_DELAY));

    // WS2812 reset pulse (>50us)
    esp_rom_delay_us(60);
}