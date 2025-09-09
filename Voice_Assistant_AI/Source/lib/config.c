#include "config.h"

const char *TAG = "HTTP";
const char *TAG_GGSHEET = "GGSHEET";
const char *TAG_GMAIL = "smtp_example";

const char file_txt[] = "/summary.txt";
const char file_wav[] = "/test06.wav";
char pathFile[64];
const char mount_point[] = "/sdcard";

int retry_wifi_count; // wifi retry count

const int headerSize = 44;
esp_err_t ret;
sdmmc_card_t *card;
FILE *file = NULL;
long file_size;