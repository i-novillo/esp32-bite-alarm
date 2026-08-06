#ifndef WS2812_H
#define WS2812_H

#include <stdint.h>

void ws2812_init(int gpio, int led_count);
void ws2812_set_pixel(int index, uint8_t r, uint8_t g, uint8_t b);
void ws2812_show(void);
void ws2812_clear(void);

#endif // WS2812_H