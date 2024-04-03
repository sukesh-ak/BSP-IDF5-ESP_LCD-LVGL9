idf_component_register(
    SRCS "wt32sc01plus.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES driver spiffs
    PRIV_REQUIRES fatfs esp_timer esp_lcd esp_lcd_touch esp_lcd_st7796
)
