# ESP-IDF 5.x + ESP_LCD + LVGL9
How to use `ESP_LCD` for Display & Touch with `LVGL 9.x` using `esp_lvgl_port` from **[Espressif Component Registry](https://components.espressif.com/)**

![Static Badge](https://img.shields.io/badge/OS-FreeRTOS-orange) ![Static Badge](https://img.shields.io/badge/SDK-ESP--IDF%20v5.x-blue) ![Static Badge](https://img.shields.io/badge/LCD%20Driver-ESP%20LCD%20ST7796%20[SPI]-red)
![Static Badge](https://img.shields.io/badge/UI%20WIDGETS-LVGL%209.x-green)  

- **LCD Driver** : [ESP LCD ST7796 SPI](https://components.espressif.com/components/espressif/esp_lcd_st7796/versions/1.2.1?language=en) - Only v1.0.0 is supported for ESP32  
- **TOUCH Driver** : [ESP LCD Touch FT5x06 Controller](https://components.espressif.com/components/espressif/esp_lcd_touch_ft5x06)  
- **UI Widgets** : [LVGL v9.x](https://components.espressif.com/components/lvgl/lvgl) with custom `lv_conf.h` => Check [CMakeLists.txt](CMakeLists.txt)  
- **ESP_LVGL_PORT** : [ESP-BSP](https://components.espressif.com/components/espressif/esp_lvgl_port) 

