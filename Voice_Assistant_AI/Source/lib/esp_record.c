#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"
#include "driver/i2s.h"

#include "esp_record.h"
#include "config.h"
#include "esp_oled.h"
void spiInit()
{
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};
    ESP_LOGI(TAG, "Initializing SD card");
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize bus.");
    }
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                          "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");
    sdmmc_card_print_info(stdout, card);
}

void i2sInit()
{
    printf("I2S is initing\n");
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_SAMPLE_BITS,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 64,
        .dma_buf_len = 1024,
        .use_apll = 1};

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);

    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = -1,
        .data_in_num = I2S_SD};

    i2s_set_pin(I2S_PORT, &pin_config);
}

void i2s_adc_data_scale(uint8_t *d_buff, uint8_t *s_buff, uint32_t len)
{
    uint32_t j = 0;
    uint16_t adc_value = 0;
    for (int i = 0; i < len; i += 2)
    {
        adc_value = ((((uint16_t)(s_buff[i + 1] & 0xf) << 8) | (s_buff[i + 0])));
        uint16_t scaled_value = (uint16_t)((uint32_t)adc_value * 65536 / 4096);
        d_buff[j++] = scaled_value & 0xFF;
        d_buff[j++] = (scaled_value >> 8) & 0xFF;
    }
}

void i2s_record(void *arg)
{
    oled_display_text("Recording start!");
    int i2s_read_len = I2S_READ_LEN;
    int flash_wr_size = 0;
    size_t bytes_read;

    char *i2s_read_buff = (char *)calloc(i2s_read_len, sizeof(char));
    uint8_t *flash_write_buff = (uint8_t *)calloc(i2s_read_len, sizeof(char));

    i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);

    oled_display_text(" Recording ...");
    ESP_LOGI(TAG, " *** Recording Start *** ");
    while (flash_wr_size < RECORD_SIZE)
    {
        // read data from I2S bus, in this case, from ADC.
        i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
        // save original dat a from I2S(ADC) into flash.
        i2s_adc_data_scale(flash_write_buff, (uint8_t *)i2s_read_buff, i2s_read_len);
        fwrite((const byte *)flash_write_buff, 1, i2s_read_len, file);
        flash_  wr_size += i2s_read_len;
        ESP_LOGI(TAG, "Sound recording (%u%%)", flash_wr_size * 100 / RECORD_SIZE);
    }
    fclose(file);

    free(i2s_read_buff);
    i2s_read_buff = NULL;
    free(flash_write_buff);
    flash_write_buff = NULL;
    ESP_LOGI(TAG, " *** Recording Done *** ");
    oled_display_text("Recording done!");
    oled_display_text("Save file wav!");
    ESP_LOGI(TAG, "Saved file %s%s", mount_point, file_wav);

    i2s_stop(I2S_PORT);
    i2s_driver_uninstall(I2S_PORT);
    vTaskDelete(NULL);
}

void wavHeader(byte *header, int wavSize)
{
    header[0] = 'R';
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';
    unsigned int fileSize = wavSize + headerSize - 8;
    header[4] = (byte)(fileSize & 0xFF);
    header[5] = (byte)((fileSize >> 8) & 0xFF);
    header[6] = (byte)((fileSize >> 16) & 0xFF);
    header[7] = (byte)((fileSize >> 24) & 0xFF);
    header[8] = 'W';
    header[9] = 'A';
    header[10] = 'V';
    header[11] = 'E';
    header[12] = 'f';
    header[13] = 'm';
    header[14] = 't';
    header[15] = ' ';
    header[16] = 0x10;
    header[17] = 0x00;
    header[18] = 0x00;
    header[19] = 0x00;
    header[20] = 0x01;
    header[21] = 0x00;
    header[22] = 0x01;
    header[23] = 0x00;
    header[24] = 0x40; // Simple rate: 8000
    header[25] = 0x1F;
    header[26] = 0x00;
    header[27] = 0x00;
    header[28] = 0x40; // Byte rate: 8000 * 2
    header[29] = 0x3E;
    header[30] = 0x00;
    header[31] = 0x00;
    header[32] = 0x01;
    header[33] = 0x00;
    header[34] = 0x10; // bits per sample
    header[35] = 0x00;
    header[36] = 'd';
    header[37] = 'a';
    header[38] = 't';
    header[39] = 'a';
    header[40] = (byte)(wavSize & 0xFF);
    header[41] = (byte)((wavSize >> 8) & 0xFF);
    header[42] = (byte)((wavSize >> 16) & 0xFF);
    header[43] = (byte)((wavSize >> 24) & 0xFF);
}
void record()
{
    spiInit();
    if (ret != ESP_OK)
    { 
        oled_display_text("SD card failed!");
        ESP_LOGE(TAG, "SD card mount failed, cannot open file.");
        return;
    }
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