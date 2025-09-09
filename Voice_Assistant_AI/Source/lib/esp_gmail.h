#ifndef ESP_GMAIL_H
#define ESP_GMAIL_H

#include "config.h"
int write_and_get_response(mbedtls_net_context *sock_fd, unsigned char *buf, size_t len);
int write_ssl_and_get_response(mbedtls_ssl_context *ssl, unsigned char *buf, size_t len);
int perform_tls_handshake(mbedtls_ssl_context *ssl);
void smtp_client_task(void *pvParameters);
#endif /* ESP_GMAIL_H */