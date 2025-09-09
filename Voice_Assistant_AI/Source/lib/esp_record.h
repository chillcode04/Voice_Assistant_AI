#ifndef ESP_RECORD_H
#define ESP_RECORD_H

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
#include "config.h"


void spiInit();
void i2sInit();
void i2s_adc_data_scale(uint8_t *d_buff, uint8_t *s_buff, uint32_t len);
void i2s_record(void *arg);
void wavHeader(byte *header, int wavSize);
void record();

#endif