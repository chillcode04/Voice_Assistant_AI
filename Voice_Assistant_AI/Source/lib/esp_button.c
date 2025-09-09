#include "config.h"
#include "esp_button.h"

const char *TAG_BUTTON = "BUTTON";

TimerHandle_t debounce_timer1;
TimerHandle_t debounce_timer2;
TimerHandle_t debounce_timer3;

uint8_t led_state1;
uint8_t led_state2;
uint8_t led_state3;

volatile bool FLAG_RECORD = false;
volatile bool FLAG_WIFI = false;
volatile bool FLAG_GMAIL = false;
int count_wifi = 0;
int count_record = 0;

void IRAM_ATTR button_isr_handler(void *arg)
{
    int gpio_num = (int)(intptr_t)arg;
    if (gpio_num == BUTTON_PIN1)
    {
        xTimerResetFromISR(debounce_timer1, NULL);
    }
    else if (gpio_num == BUTTON_PIN2)
    {
        xTimerResetFromISR(debounce_timer2, NULL);
    }
    else if (gpio_num == BUTTON_PIN3)
    {
        xTimerResetFromISR(debounce_timer3, NULL);
    }
}

/** Callback debounce cho nút 1 */
void debounce_timer_callback1(TimerHandle_t Timer)
{
    if (gpio_get_level(BUTTON_PIN1) == 0)
    {
        led_state1 = !led_state1;
        FLAG_RECORD = true;
        count_record++;
        ESP_LOGI(TAG_BUTTON, "Button 1 Pressed! LED1: %s", led_state1 ? "ON" : "OFF");
    }
}

/** Callback debounce cho nút 2 */
void debounce_timer_callback2(TimerHandle_t Timer)
{
    if (gpio_get_level(BUTTON_PIN2) == 0)
    {
        led_state2 = !led_state2;
        FLAG_WIFI = true;
        count_wifi++;
        //ESP_LOGI(TAG_BUTTON, "Button 2 Pressed! LED2: %s", led_state2 ? "ON" : "OFF");
    }
}

/** Callback debounce cho nút 2 */
void debounce_timer_callback3(TimerHandle_t Timer)
{
    if (gpio_get_level(BUTTON_PIN3) == 0)
    {
        led_state3 = !led_state3;
        FLAG_GMAIL = true;
        //ESP_LOGI(TAG_BUTTON, "Button 3 Pressed! LED3: %s", led_state3 ? "ON" : "OFF");
    }
}

void config_button()
{
    // Cấu hình nút nhấn 1
    gpio_set_direction(BUTTON_PIN1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN1, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BUTTON_PIN1, GPIO_INTR_NEGEDGE);

    // Cấu hình nút nhấn 2
    gpio_set_direction(BUTTON_PIN2, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN2, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BUTTON_PIN2, GPIO_INTR_NEGEDGE);

    // Cấu hình nút nhấn 3
    gpio_set_direction(BUTTON_PIN3, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN3, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BUTTON_PIN3, GPIO_INTR_NEGEDGE);

    // Tạo timer debounce
    debounce_timer1 = xTimerCreate("debounce_timer1", pdMS_TO_TICKS(DEBOUNCE_TIME_MS), pdFALSE, NULL, debounce_timer_callback1);
    debounce_timer2 = xTimerCreate("debounce_timer2", pdMS_TO_TICKS(DEBOUNCE_TIME_MS), pdFALSE, NULL, debounce_timer_callback2);
    debounce_timer3 = xTimerCreate("debounce_timer3", pdMS_TO_TICKS(DEBOUNCE_TIME_MS), pdFALSE, NULL, debounce_timer_callback3);
    // Cài đặt ISR
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN1, button_isr_handler, (void *)(intptr_t)BUTTON_PIN1);
    gpio_isr_handler_add(BUTTON_PIN2, button_isr_handler, (void *)(intptr_t)BUTTON_PIN2);
    gpio_isr_handler_add(BUTTON_PIN3, button_isr_handler, (void *)(intptr_t)BUTTON_PIN3);

    ESP_LOGI(TAG_BUTTON, "3 Buttons ISR with Debounce Installed!");
}