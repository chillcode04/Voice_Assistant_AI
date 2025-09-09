#include "config.h"
#include "esp_upload.h"
#include "esp_oled.h"

esp_err_t save_file_txt(const char *response)
{
    sprintf(pathFile, "%s%s", mount_point, file_txt);
    FILE *file = fopen(pathFile, "w, ccs=UTF-8");
    if (file == NULL)
    {
        ESP_LOGE(TAG_GGSHEET, "Failed to open file: %s: %s (errno: %d)", file_txt, strerror(errno), errno);
        return ESP_FAIL;
    }
    // Ghi dữ liệu vào file
    fprintf(file, "%s", response);
    fclose(file);
    printf("Saved file %s\n", file_txt);
    return ESP_OK;
}

char content[1024];

void write_to_ggsheet()
{
    oled_display_text("Sending GGsheet...");
    esp_http_client_config_t generate_config = {
        .url = "https://script.google.com/macros/s/AKfycbywtq-Jz3v5qYTirXlF9r6extykvq7wduu8SR7cByMpOilj0rfhSXt4k2D8syaTHxoCsQ/exec",
        .method = HTTP_METHOD_POST,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .buffer_size = 8192,
        .timeout_ms = 50000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&generate_config);
    char json_payload[1024];
    int written = snprintf(json_payload, sizeof(json_payload),
                           "{\"content\": \"%s\"}", content);
    printf("JSON payload: %s\n", json_payload);
    if (written < 0 || written >= sizeof(json_payload))
    {
        printf("%d\n",written);
        ESP_LOGE(TAG_GGSHEET, "JSON payload buffer overflow or error");
        esp_http_client_cleanup(client);
        return;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_err_t err = esp_http_client_open(client, strlen(json_payload));
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_GGSHEET, "Failed to open HTTP connection for generateContent: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return;
    }

    int bytes_written = esp_http_client_write(client, json_payload, strlen(json_payload));
    if (bytes_written < 0)
    {
        ESP_LOGE(TAG_GGSHEET, "Failed to write data for generateContent");
        esp_http_client_cleanup(client);
        return;
    }
    ESP_LOGI(TAG_GGSHEET, "Wrote %d bytes for generateContent", bytes_written);

    esp_http_client_fetch_headers(client);
    int generate_status = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG_GGSHEET, "HTTP POST GGSHEET Status = %d", generate_status);
    int status_code = esp_http_client_get_status_code(client);
    if (status_code == 302)
    {
        oled_display_text("Sent GGsheet!");

        printf("Sent google sheet\n");
    }

    esp_http_client_cleanup(client);
}
// Ham giữ nguyên /n và /r trong chuỗi
void simple_escape_newlines(char *str)
{
    char temp[1024 * 2];
    int i = 0, j = 0;
    while (str[i] != '\0' && j < sizeof(temp) - 1)
    {
        if (str[i] == '\n')
        {
            temp[j++] = '\\';
            temp[j++] = 'n';
        }
        else if (str[i] == '\r')
        {
            temp[j++] = '\\';
            temp[j++] = 'r';
        }
        else
        {
            temp[j++] = str[i];
        }
        i++;
    }
    temp[j] = '\0';
    strcpy(str, temp);
}

esp_err_t client_event_post_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("SERVER_RESPONE:\n ==================== Response ====================\n");
        printf("%.*s", evt->data_len, (char *)evt->data);
        printf("\n====================      End      =====================\n");
        strncpy(content, (char *)evt->data, evt->data_len);
        content[evt->data_len - 1] = '\0';
        simple_escape_newlines(content);
        write_to_ggsheet();
        save_file_txt(evt->data);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP request finished");
        break;
    default:
        break;
    }
    return ESP_OK;
}

typedef struct
{
    int task_index;
    size_t start_offset;
    size_t part_size;
    const char *file_path;
} TaskInfo;

void upload_task(void *pvParameters)
{
    int retry_count_per_task = 0;
    TaskInfo *info = (TaskInfo *)pvParameters;
sendback:
    if (retry_count_per_task < MAX_RETRY_PER_TASK)
    {
        printf("Task %d started,file_path %s offset: %zu, size: %zu\n", info->task_index, info->file_path, info->start_offset, info->part_size);
        FILE *file = fopen(info->file_path, "rb");
        if (!file)
        {
            ESP_LOGE(TAG, "Failed to open file %s: %s", info->file_path, strerror(errno));
            goto cleanup;
        }

        // Di chuyển đến offset của task
        fseek(file, info->start_offset, SEEK_SET);

        esp_http_client_config_t config = {
            .url = "http://192.168.94.34:8888/uploadAudio",
            .method = HTTP_METHOD_POST,
            .buffer_size = 8192,
            .timeout_ms = 120000,
            .event_handler = client_event_post_handler};
        esp_http_client_handle_t client = esp_http_client_init(&config);

        // Set headers
        esp_http_client_set_header(client, "Content-Type", "audio/wav");

        char task_index_str[16];
        snprintf(task_index_str, sizeof(task_index_str), "%d", info->task_index);

        esp_http_client_set_header(client, "X-Task-x", task_index_str);
        esp_http_client_set_header(client, "X-File-Name", info->file_path);

        char content_length_str[16];
        snprintf(content_length_str, sizeof(content_length_str), "%zu", info->part_size);
        esp_http_client_set_header(client, "Content-Length", content_length_str);

        esp_err_t err = esp_http_client_open(client, info->part_size);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to open HTTP for task %d: %s", info->task_index, esp_err_to_name(err));
            fclose(file);
            esp_http_client_cleanup(client);
            goto cleanup;
        }

        char buffer[BUFFER_SIZE];
        size_t total_bytes_sent = 0;
        size_t bytes_read;
        while (total_bytes_sent < info->part_size)
        {
            size_t bytes_to_read = (info->part_size - total_bytes_sent) < BUFFER_SIZE ? (info->part_size - total_bytes_sent) : BUFFER_SIZE;
            bytes_read = fread(buffer, 1, bytes_to_read, file);
            if (bytes_read == 0)
                break;

            int retry_count_per_chunk = 0;
            int bytes_written = 0;

            while (retry_count_per_chunk < MAX_RETRY_PER_CHUNK)
            {
                bytes_written = esp_http_client_write(client, buffer, bytes_read);
                if (bytes_written < bytes_read)
                {
                    ESP_LOGE(TAG, "Failed to write for task %d, retrying... (%d/%d)", info->task_index, retry_count_per_chunk + 1, MAX_RETRY_PER_CHUNK);
                    retry_count_per_chunk++;
                    vTaskDelay(100 / portTICK_PERIOD_MS); // Delay before retry
                }
                else
                {
                    break; // Break the loop if write was successful
                }
            }
            if (retry_count_per_chunk == MAX_RETRY_PER_CHUNK)
            {
                esp_http_client_cleanup(client);
                retry_count_per_task++;
                goto sendback;
            }
            total_bytes_sent += bytes_written;
            ESP_LOGI(TAG, "Task %d sent %d/%u bytes", info->task_index, total_bytes_sent, info->part_size);
        }

        esp_http_client_fetch_headers(client);
        int status_code = esp_http_client_get_status_code(client);

        if (status_code != 200)
        {
            esp_http_client_cleanup(client);
            retry_count_per_task++;
            goto sendback;
        }
        fclose(file);
        esp_http_client_cleanup(client);
    }
    else
    {
        ESP_LOGE(TAG, "Task %d failed after %d retries", info->task_index, MAX_RETRY_PER_TASK);
        oled_display_text("Upload failed");
    }

cleanup:
    free(info);
    vTaskDelete(NULL);
}
void upload()
{
    oled_display_text("Uploading ...");
    sprintf(pathFile, "%s%s", mount_point, file_wav);
    FILE *file = fopen(pathFile, "rb");
    if (!file)
    {
        ESP_LOGE(TAG, "Failed to open file %s: %s", pathFile, strerror(errno));
        vTaskDelete(NULL);
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    fclose(file);

    ESP_LOGI(TAG, "Audio File found, size: %ld bytes", file_size);

    size_t part_size = file_size / NUM_TASKS;                        // kich thuoc moi file
    size_t last_part_size = file_size - part_size * (NUM_TASKS - 1); // Phần cuối cùng

    for (int i = 0; i < NUM_TASKS; i++)
    {
        TaskInfo *info = malloc(sizeof(TaskInfo));
        if (!info)
        {
            ESP_LOGE(TAG, "Failed to allocate task info");
            continue;
        }

        info->task_index = i;
        info->start_offset = i * part_size;
        info->part_size = (i == NUM_TASKS - 1) ? last_part_size : part_size;
        info->file_path = pathFile;

        char task_name[16];
        snprintf(task_name, sizeof(task_name), "upload_%d", i);
        xTaskCreatePinnedToCore(&upload_task, task_name, 1024 * 16, info, 5, NULL, 0);
    }

    vTaskDelete(NULL);
}

void post_task(void *arg)
{
    upload();
}
void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        oled_display_text(" WiFi connecting ...");
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected to ap SSID: %s password: %s\n", WIFI_SSID, WIFI_PASS);
        oled_display_text(" WiFi connected");

        retry_wifi_count = 0;
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        if (retry_wifi_count < MAX_RETRY)
        {
            retry_wifi_count++;
            printf("Reconnecting to WiFi... Attempt %d/%d\n", retry_wifi_count, MAX_RETRY);
            esp_wifi_connect();
        }
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        xTaskCreate(post_task, "post_task", 1024 * 16, NULL, 3, NULL);
        break;
    default:
        break;
    }
}

void wifi_connection()
{
    // 1 - Wi-Fi/LwIP Init Phase
    esp_netif_init();                    // TCP/IP initiation 					s1.1
    esp_event_loop_create_default();     // event loop 			                s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    s1.4

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    esp_wifi_set_protocol(WIFI_IF_STA, 
    WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_11AX);
    esp_wifi_start();
    esp_wifi_connect();
}

void connect_wifi()
{
    nvs_flash_init();
    wifi_connection();
}