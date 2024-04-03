idf_component_register(SRCS "main.c" "main_ui.c" "emoji.c"
                    
                    INCLUDE_DIRS ".")

                    spiffs_create_partition_image(storage ${PROJECT_DIR}/spiff FLASH_IN_PROJECT)