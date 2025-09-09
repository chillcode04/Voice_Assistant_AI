#ifndef ESP_OLED_H_
#define ESP_OLED_H_

void i2c_master_init();
void sh1106_init();
void task_sh1106_display_clear(void *ignore);
void task_sh1106_display_text(const void *arg_text);
void oled_config();
void oled_display_text(const void *arg_text);
    
#endif /* ESP_OLED_H_ */

