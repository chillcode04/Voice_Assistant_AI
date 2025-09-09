#ifndef ESP_UPLOAD_H
#define ESP_UPLOAD_H
#include   "config.h"

esp_err_t save_file_txt(const char *response);
void write_to_ggsheet();
void simple_escape_newlines(char *str);
esp_err_t client_event_post_handler(esp_http_client_event_handle_t evt);
void upload_task(void *pvParameters);
void upload();
void post_task(void *arg);
void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void wifi_connection();
void connect_wifi();
#endif /* ESP_UPLOAD_H */