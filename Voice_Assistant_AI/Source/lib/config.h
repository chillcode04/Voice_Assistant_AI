#ifndef CONFIG_H
#define CONFIG_H
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"
#include "driver/i2s.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_http_client.h"

#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include <mbedtls/base64.h>
#include <sys/param.h>

extern const char *TAG;
extern const char *TAG_GGSHEET;
extern const char mount_point[];

/*----------------------------------------WIFI--------------------------------------------------------*/
// parameters for WIFI
#define WIFI_SSID "NQH"
#define WIFI_PASS "12345679"
// #define WIFI_SSID "Phuong"
// #define WIFI_PASS "12345678"
#define MAX_RETRY 5
extern int retry_wifi_count;

//----------------------------------------FILE---------------------------------------------------------//
typedef uint8_t byte;
extern const char file_txt[];
extern const char file_wav[];
extern char pathFile[64];
extern FILE *file;
extern const int headerSize;
extern esp_err_t ret;
extern sdmmc_card_t *card;
/*---------------------------------------I2S-------------------------------------------------------------*/
// I2S peripheral
#define I2S_WS 10
#define I2S_SD 9
#define I2S_SCK 12
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE (8000)
#define I2S_SAMPLE_BITS (16)
#define I2S_READ_LEN (16 * 1024)
#define RECORD_TIME (10 * 60) 
#define I2S_CHANNEL_NUM (1)
#define RECORD_SIZE (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORD_TIME)

/*---------------------------------------SPI--------------------------------------------------------------*/
// SPI peripheral
// #define PIN_NUM_MISO 6
// #define PIN_NUM_MOSI 5
// #define PIN_NUM_CLK 4
// #define PIN_NUM_CS 7
// #define SPI_DMA_CHAN SPI_DMA_CH_AUTOD

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 5
#define SPI_DMA_CHAN SPI_DMA_CH_AUTO
//----------------------------------------GMAIL--------------------------------------------------/
//* Constants that are configurable in menuconfig */
#define MAIL_SERVER "smtp.googlemail.com"
#define MAIL_PORT "587"
#define SENDER_MAIL "ahquyendz2018@gmail.com"
#define SENDER_PASSWORD "xldc otjo pbkh pdnt"
#define RECIPIENT_MAIL "ahhuy2021p@gmail.com"

extern const char *TAG_GMAIL;
#define BUF_SIZE 512
#define VALIDATE_MBEDTLS_RETURN(ret, min_valid_ret, max_valid_ret, goto_label) \
    do                                                                         \
    {                                                                          \
        if (ret < min_valid_ret || ret > max_valid_ret)                        \
        {                                                                      \
            goto goto_label;                                                   \
        }                                                                      \
    } while (0)


extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[] asm("_binary_server_root_cert_pem_end");

//-----------------------------------------BUTTON--------------------------------------------------/
#define BUTTON_PIN1 13
#define BUTTON_PIN2 3
#define BUTTON_PIN3 2
#define DEBOUNCE_TIME_MS 50
extern const char *TAG_BUTTON;

extern TimerHandle_t debounce_timer1;
extern TimerHandle_t debounce_timer2;
extern TimerHandle_t debounce_timer3;

extern uint8_t led_state1;
extern uint8_t led_state2;
extern uint8_t led_state3;

extern volatile bool FLAG_RECORD;
extern volatile bool FLAG_WIFI;
extern volatile bool FLAG_GMAIL;
extern int count_wifi;
extern int count_record;
// -----------------------------------UPLOAD-------------------------------------------------/
#define BUFFER_SIZE (1024 * 10) 
#define NUM_TASKS 4   

#define MAX_RETRY_PER_CHUNK 3
#define MAX_RETRY_PER_TASK 3
extern long file_size;
//----------------------------------OLED-------------------------------------------------------/
#define SDA_PIN 21
#define SCL_PIN 22

#define TAG_OLED "OLED"
#endif