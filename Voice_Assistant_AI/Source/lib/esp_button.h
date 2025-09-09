#ifndef ESP_BUTTON_H
#define ESP_BUTTON_H

#include "config.h"



void IRAM_ATTR button_isr_handler(void *arg);
void debounce_timer_callback1(TimerHandle_t Timer);
void debounce_timer_callback2(TimerHandle_t Timer);
void debounce_timer_callback3(TimerHandle_t Timer);
void config_button();
#endif /* ESP_BUTTON_H */