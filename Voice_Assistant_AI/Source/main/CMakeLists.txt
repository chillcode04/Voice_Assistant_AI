idf_component_register(SRCS "main.c" "../lib/esp_oled.c" "../lib/config.c" "../lib/esp_record.c" "../lib/esp_gmail.c"
                    "../lib/esp_upload.c" "../lib/esp_button.c"
                    INCLUDE_DIRS "" "../lib" 
                    EMBED_TXTFILES server_root_cert.pem)
