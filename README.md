# BSP (Board Support Package) using ESP-IDF 5.x + ESP_LCD + LVGL 9.x
How to use `ESP_LCD` components for Display & Touch with `LVGL 9.x` using `ESP_LVGL_PORT` from `ESP-BSP`.  
**[Espressif Component Registry](https://components.espressif.com/)**


## BSP for Wireless Tag WT32-SC01 Plus [ESP32-S3]:  
[![Purchase link](https://img.shields.io/static/v1?label=Click%20here%20to%20Purchase&message=WT32-SC01%20PLUS&logo=alibabadotcom&color=00A226&style=for-the-badge)](https://bit.ly/wt32-sc01-plus) 

![Static Badge](https://img.shields.io/badge/DEVICE-WT32--SC01%20Plus-8A2BE2) ![Static Badge](https://img.shields.io/badge/MCU-ESP32--S3-8A2BE2) ![Static Badge](https://img.shields.io/badge/OS-FreeRTOS-orange) ![Static Badge](https://img.shields.io/badge/SDK-ESP--IDF%20v5.x-blue) ![Static Badge](https://img.shields.io/badge/UI%20WIDGETS-LVGL%209.x-green)  
![Static Badge](https://img.shields.io/badge/LCD%20Driver-ESP%20LCD%20ST7796%20[8Bit%20Parellel]-red)  ![Static Badge](https://img.shields.io/badge/TOUCH%20Driver-FT5x06-00FFFF) 


Supported features:
- Display
- Capacitive Touch
- SPIFFS
- uSD card
- LVGL 9.x with lv_Observer 

Dependencies:
- **LCD Driver** : [ESP LCD ST7796 8Bit Parellel](https://components.espressif.com/components/espressif/esp_lcd_st7796/versions/1.2.1?language=en)
- **TOUCH Driver** : [ESP LCD Touch FT5x06 Controller](https://components.espressif.com/components/espressif/esp_lcd_touch_ft5x06)  
- **UI Widgets** : [LVGL v9.x](https://components.espressif.com/components/lvgl/lvgl) with custom `lv_conf.h` => Check [CMakeLists.txt](CMakeLists.txt)  
- **ESP_LVGL_PORT** : [ESP-BSP](https://components.espressif.com/components/espressif/esp_lvgl_port) 

```bash
# Set the current device target to ESP32-S3
idf.py set-target esp32s3

# Build with "WT32-SC01 Plus" specific configuration
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.wt32sc01plus" build
idf.py -p <PORT> flash monitor
```


##
[![Github Sponsor](https://img.shields.io/badge/label-%E2%9D%A4-FF007F?style=for-the-badge&logo=github&label=CLICK%20HERE%20TO%20SPONSOR&labelColor=blue&color=FF007F
)](https://github.com/sponsors/sukesh-ak)    

If you use my projects or like them, consider sponsoring me. Anything helps and encourages me to keep going.  
See details here: https://github.com/sponsors/sukesh-ak  

Your sponsorship would help me not only to maintain the projects I'm involved in, but also support my other community activities and purchase hardware for testing these libraries. If you're an individual user who has enjoyed my projects or benefited from my community work, please consider donating as a sign of appreciation. If you run a business that uses my work in your products, sponsoring my development makes good business sense: it ensures that the projects your product relies on stay healthy and actively maintained.

Thank you for considering supporting my work!

## BSP for Wireless Tag WT32-SC01 [ESP32] (Work in progress):  
![Static Badge](https://img.shields.io/badge/DEVICE-WT32--SC01-8A2BE2) ![Static Badge](https://img.shields.io/badge/MCU-ESP32-8A2BE2) 
![Static Badge](https://img.shields.io/badge/OS-FreeRTOS-orange) ![Static Badge](https://img.shields.io/badge/SDK-ESP--IDF%20v5.x-blue) ![Static Badge](https://img.shields.io/badge/UI%20WIDGETS-LVGL%209.x-green)   
![Static Badge](https://img.shields.io/badge/LCD%20Driver-ESP%20LCD%20ST7796%20[SPI]-red) ![Static Badge](https://img.shields.io/badge/TOUCH%20Driver-FT5x06-00FFFF)
   

- **LCD Driver** : [ESP LCD ST7796 SPI](https://components.espressif.com/components/espressif/esp_lcd_st7796/versions/1.2.1?language=en) - Only v1.0.0 is supported for ESP32  
- **TOUCH Driver** : [ESP LCD Touch FT5x06 Controller](https://components.espressif.com/components/espressif/esp_lcd_touch_ft5x06)  
- **UI Widgets** : [LVGL v9.x](https://components.espressif.com/components/lvgl/lvgl) with custom `lv_conf.h` => Check [CMakeLists.txt](CMakeLists.txt)  
- **ESP_LVGL_PORT** : [ESP-BSP](https://components.espressif.com/components/espressif/esp_lvgl_port) 

```bash
# Build with "WT32-SC01" specific configuration
idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.wt32sc01" build
idf.py -p <PORT> flash monitor
```
## BSP TODO LIST
- [ ] Makerfabs - MaTouch_ESP32-S3 Parallel IPS with Touch 4.3" 800x400 ST7282
- [ ] Makerfabs - MaTouch_ESP32-S3 Parallel IPS with Touch 4.0" 480*480 ST7701
- [ ] Makerfabs - MaTouch_ESP32-S3 Parallel TFT with Touch 3.5" 480x320 ILI9488
- [ ] Makerfabs - MaTouch_ESP32-S3 SPI TFT with Touch 3.5" 480x320 ILI9488
