#include "hmi_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"

#define TAG "HMI_MANAGER"

#define KY_040_CLK_PIN GPIO_NUM_33
#define KY_040_DT_PIN GPIO_NUM_32
#define KY_040_BUTTON_PIN GPIO_NUM_25

#define SENSITIVITY_QUEUE_SIZE 20 // TODO: Make configurable
#define SENSITIVITY_QUEUE_TIMEOUT 1000 // TODO: Make configurable
#define SENSITIVITY_DEBOUNCE_TIME 5000 // microseconds TODO: Make configurable
#define BUTTON_DEBOUNCE_TIME 10000 // microseconds TODO: Make configurable

static sensitivity_level_t s_current_sensitivity = SENS_LEVEL_5;
static QueueHandle_t s_hmi_event_queue = NULL;
static int sensitivity_change_ctr = 0; // This value is used to adjust how many encoder clicks are needed to increase sensitivity TODO: Make configurable.
static volatile int64_t last_sensitivity_interrupt_time = 0;
static volatile int64_t last_button_interrupt_time = 0;

typedef enum {
    SENS_INCREASED = 0,
    SENS_DECREASED = 1,
    BUTTON_PRESSED = 2
} hmi_event_t;

static void IRAM_ATTR sensitivity_input_handler(void *arg)
{
    int64_t now = esp_timer_get_time();

    if (now - last_sensitivity_interrupt_time < SENSITIVITY_DEBOUNCE_TIME) return;
    last_sensitivity_interrupt_time = now;

    if (s_hmi_event_queue == NULL) {
        return;
    }

    int clk = gpio_get_level(KY_040_CLK_PIN);
    int dt = gpio_get_level(KY_040_DT_PIN);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    hmi_event_t event = (clk != dt) ? SENS_INCREASED : SENS_DECREASED;
    xQueueSendFromISR(s_hmi_event_queue, &event, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

static void IRAM_ATTR button_input_handler(void *arg)
{
    int64_t now = esp_timer_get_time();

    if (now - last_button_interrupt_time < BUTTON_DEBOUNCE_TIME) return;
    last_button_interrupt_time = now;

    if (s_hmi_event_queue == NULL) {
        return;
    }

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    hmi_event_t event = BUTTON_PRESSED;
    xQueueSendFromISR(s_hmi_event_queue, &event, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void setup_hmi_manager(void)
{ 
    gpio_config_t clk_io_conf = {
        .pin_bit_mask = (1ULL << KY_040_CLK_PIN),    // Select GPIO 33
        .mode = GPIO_MODE_INPUT,                     // Set as input
        .pull_up_en = GPIO_PULLUP_ENABLE,            // Enable internal pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE,       // Disable pull-down
        .intr_type = GPIO_INTR_POSEDGE               // Enbable raising edge interrupt
    };
    gpio_config(&clk_io_conf);

    gpio_config_t dt_io_conf = {
        .pin_bit_mask = (1ULL << KY_040_DT_PIN),    // Select GPIO 32
        .mode = GPIO_MODE_INPUT,                    // Set as input
        .pull_up_en = GPIO_PULLUP_ENABLE,           // Enable internal pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE,      // Disable pull-down
        .intr_type = GPIO_INTR_DISABLE              // Disable interrupts
    };
    gpio_config(&dt_io_conf);

    gpio_config_t button_io_conf = {
        .pin_bit_mask = (1ULL << KY_040_BUTTON_PIN),    // Select GPIO 33
        .mode = GPIO_MODE_INPUT,                        // Set as input
        .pull_up_en = GPIO_PULLUP_ENABLE,               // Enable internal pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE,          // Disable pull-down
        .intr_type = GPIO_INTR_NEGEDGE                  // Enbable raising edge interrupt
    };
    gpio_config(&button_io_conf);
    
    s_hmi_event_queue = xQueueCreate(SENSITIVITY_QUEUE_SIZE, sizeof(hmi_event_t)); // TODO: Handle error when creating
}

static void xHMIManagerTask(void *pvParameters)
{
    while (1) {
        hmi_event_t event;
        if (xQueueReceive(s_hmi_event_queue, &event, pdMS_TO_TICKS(SENSITIVITY_QUEUE_TIMEOUT)) == pdPASS) {
            if (event == BUTTON_PRESSED) {
                ESP_LOGI(TAG, "Button pressed"); // TODO: Make debug
            }
            else {
                //ESP_LOGI(TAG, "Received sensitivity event: %d", event); // TODO: Make debug
                sensitivity_change_ctr += (event == SENS_INCREASED) ? 1 : -1;

                if (sensitivity_change_ctr >= 2) {
                    if (s_current_sensitivity < SENS_LEVEL_10)
                    {
                        s_current_sensitivity++;
                        ESP_LOGI(TAG, "Sensitivity increased to level %d", s_current_sensitivity + 1);
                    }
                    sensitivity_change_ctr = 0;
                }
                else if (sensitivity_change_ctr <= -2) {
                    if (s_current_sensitivity > SENS_LEVEL_1)
                    {
                        s_current_sensitivity--;
                        ESP_LOGI(TAG, "Sensitivity decreased to level %d", s_current_sensitivity + 1);
                    }
                    sensitivity_change_ctr = 0;
                }
            }
        }
    }
}

void start_hmi_task(void)
{
    gpio_install_isr_service(0);
    gpio_isr_handler_add(KY_040_CLK_PIN, sensitivity_input_handler, NULL);
    gpio_isr_handler_add(KY_040_BUTTON_PIN, button_input_handler, NULL);
    xTaskCreate(xHMIManagerTask, "HMI Manager Task", 4096, NULL, 1, NULL); // TODO: Make sensitivity manager task priority configurable
}


sensitivity_level_t get_sensitivity(void)
{
    return s_current_sensitivity;
}