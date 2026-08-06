#include "buzzer_manager.h"

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define BUZZER_PIN 4 // TODO: Make configurable
#define PWM_DUTY 512 // TODO: Make configurable

#define STATE_MACHINE_NOTIFICATION_TIMEOUT   50     // TODO: Make configurable
#define BUZZER_ON_POSSIBLE_BITE_FREQ         1000   // TODO: Make configurable
#define BUZZER_ON_POSSIBLE_BITE_DURATION     100    // TODO: Make configurable
#define BUZZER_ON_ALARM_FREQ                 1700   // TODO: Make configurable
#define BUZZER_ON_ALARM_DURATION             100    // TODO: Make configurable
#define BUZZER_OFF_ALARM_DURATION            25     // TODO: Make configurable

static TaskHandle_t s_buzzer_task_handle = NULL;

void setup_buzzer_manager(void)
{
    // Configure PWM timer
    ledc_timer_config_t timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_10_BIT,
        .freq_hz          = BUZZER_ON_POSSIBLE_BITE_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    // Configure PWM channel
    ledc_channel_config_t channel = {
        .gpio_num   = BUZZER_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = PWM_DUTY,
        .hpoint     = 0
    };
    ledc_channel_config(&channel);
}

static void buzzer_on(uint32_t freq_hz)
{
    ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq_hz);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, PWM_DUTY);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

static void buzzer_off(void)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

static void xTaskBuzzerManager(void *pvParameters)
{
    uint32_t notified_status;
    buzzer_status_t current_status = BUZZER_OFF;

    TickType_t beep_on_off_time = 0;
    bool beep_active = false;

    while(1) {

        if (xTaskNotifyWait(0, 0xFFFFFFFF, &notified_status, pdMS_TO_TICKS(20))) {
            current_status = (buzzer_status_t)notified_status;
            beep_active = false;
            beep_on_off_time = xTaskGetTickCount();

            if (current_status == BUZZER_OFF) {
                buzzer_off();
            }
        }

        switch (current_status)
        {
            case BUZZER_OFF:
                break;

            case BUZZER_ON_POSSIBLE_BITE:
                if (!beep_active) {
                    buzzer_on(BUZZER_ON_POSSIBLE_BITE_FREQ);
                    beep_on_off_time = xTaskGetTickCount();
                    beep_active = true;
                } else if (xTaskGetTickCount() - beep_on_off_time >= pdMS_TO_TICKS(BUZZER_ON_POSSIBLE_BITE_DURATION)) {
                    buzzer_off();
                    current_status = BUZZER_OFF;
                    beep_active = false;
                }
                break;

            case BUZZER_ON_ALARM:
                TickType_t now = xTaskGetTickCount();

                if (beep_active) {
                    if (now - beep_on_off_time >= pdMS_TO_TICKS(BUZZER_ON_ALARM_DURATION)) {
                        buzzer_off();
                        beep_on_off_time = now;
                        beep_active = false;
                    }
                } else {
                    if (now - beep_on_off_time >= pdMS_TO_TICKS(BUZZER_OFF_ALARM_DURATION)) {
                        buzzer_on(BUZZER_ON_ALARM_FREQ);
                        beep_on_off_time = now;
                        beep_active = true;
                    }
                }
                break;

            default:
                break;
        }
    }
}

void start_buzzer_task(void)
{
    xTaskCreate(xTaskBuzzerManager, "Buzzer Manager Task", 4096, NULL, 3, &s_buzzer_task_handle);
}

void buzzer_set_state(buzzer_status_t state)
{
    if (s_buzzer_task_handle != NULL) {
        xTaskNotify(
            s_buzzer_task_handle,
            state,
            eSetValueWithOverwrite
        );
    }
}