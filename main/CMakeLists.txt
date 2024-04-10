file(GLOB_RECURSE FONTS_SOURCES fonts/*.c)
file(GLOB_RECURSE IMAGES_SOURCES images/*.c) 

idf_component_register(SRCS "main.c" 
    "main_ui.c"
    ${FONTS_SOURCES} 
    ${IMAGES_SOURCES}
    INCLUDE_DIRS ".")
    spiffs_create_partition_image(storage ${PROJECT_DIR}/spiff FLASH_IN_PROJECT)