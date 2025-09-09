#include "esp_oled.h"
#include "config.h"
#include "esp_record.h"
#include "esp_gmail.h"
#include "esp_upload.h"
#include "esp_button.h"

void app_main(void)
{
    oled_config();
    oled_display_text("SANSLAB");
    config_button();
    while (1)
    {
        if (FLAG_RECORD == true)
        {
            FLAG_RECORD = false;
            if (count_record <= 1)
            {
                record();
            }
            else
            {
                sprintf(pathFile, "%s/%s", mount_point, file_wav);
                file = fopen(pathFile, "wb");
                if (file == NULL)
                {
                    ESP_LOGE(TAG, "Failed to open file: %s", strerror(errno));
                }
                byte header[headerSize];
                wavHeader(header, RECORD_SIZE);
                fwrite(header, 1, headerSize, file);

                i2sInit();
                xTaskCreate(i2s_record, "i2s_record", 1024 * 25, NULL, 3, NULL);
            }
        }
        if (FLAG_WIFI == true)
        {
            FLAG_WIFI = false;
            if (count_record < 1)
            {
                spiInit();
                if (ret != ESP_OK)
                {
                    ESP_LOGE(TAG, "SD card mount failed, cannot open file.");
                    return;
                }
                count_record++;
            }

            if (count_wifi <= 1)
            {
                connect_wifi();
            }
            else
            {
                xTaskCreate(post_task, "post_task", 1024 * 16, NULL, 3, NULL);
            }
        }
        if (FLAG_GMAIL == true)
        {
            FLAG_GMAIL = false;
            if (count_wifi < 1)
            {
                connect_wifi();
            }
            if (count_record < 1)
            {
                spiInit();
                if (ret != ESP_OK)
                {
                    ESP_LOGE(TAG, "SD card mount failed, cannot open file.");
                    return;
                }
            }
            xTaskCreate(&smtp_client_task, "smtp_client_task", 1024 * 16, NULL, 5, NULL);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
