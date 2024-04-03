/**
 * @file
 * @brief ESP BSP: WT32-SC01 Plus (ESP32-S3)
 */
#pragma once

#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/sdmmc_host.h"
#include "soc/usb_pins.h"
#include "bsp/config.h"
#include "bsp/display.h"
#include "bsp/touch.h"
#include "driver/i2s_std.h"

#include "lvgl.h"
#include "esp_lvgl_port.h"

/**************************************************************************************************
 *  BSP Capabilities
 **************************************************************************************************/

#define BSP_CAPS_DISPLAY        1
#define BSP_CAPS_TOUCH          1
#define BSP_CAPS_BUTTONS        1
#define BSP_CAPS_AUDIO          1
#define BSP_CAPS_AUDIO_SPEAKER  1
#define BSP_CAPS_AUDIO_MIC      0
#define BSP_CAPS_SDCARD         1
#define BSP_CAPS_IMU            0

/* I2C */
#define BSP_I2C_SCL           (GPIO_NUM_5)
#define BSP_I2C_SDA           (GPIO_NUM_6)

/* Display - ST7789 8 Bit parellel */
#define BSP_LCD_WIDTH         (8) 
#define BSP_LCD_DATA0         (GPIO_NUM_9)
#define BSP_LCD_DATA1         (GPIO_NUM_46)
#define BSP_LCD_DATA2         (GPIO_NUM_3)
#define BSP_LCD_DATA3         (GPIO_NUM_8)
#define BSP_LCD_DATA4         (GPIO_NUM_18)
#define BSP_LCD_DATA5         (GPIO_NUM_17)
#define BSP_LCD_DATA6         (GPIO_NUM_16)
#define BSP_LCD_DATA7         (GPIO_NUM_15)

#define BSP_LCD_CS           (GPIO_NUM_NC) 
#define BSP_LCD_DC           (GPIO_NUM_0)  
#define BSP_LCD_WR           (GPIO_NUM_47) 
#define BSP_LCD_RD           (GPIO_NUM_NC) 
#define BSP_LCD_RST          (GPIO_NUM_4)  
#define BSP_LCD_TE           (GPIO_NUM_48) 
#define BSP_LCD_BACKLIGHT    (GPIO_NUM_45) 
#define BSP_LCD_TP_INT       (GPIO_NUM_7)  
#define BSP_LCD_TP_RST       (GPIO_NUM_NC) 

#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

/* Display Brightness */
#define LCD_LEDC_CH            1 //CONFIG_BSP_DISPLAY_BRIGHTNESS_LEDC_CH

/* SD card */
#define BSP_SD_MOSI           (GPIO_NUM_40)
#define BSP_SD_MISO           (GPIO_NUM_38)
#define BSP_SD_SCLK           (GPIO_NUM_39)
#define BSP_SD_CS             (GPIO_NUM_41)

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bsp_i2c_init(void);
esp_err_t bsp_i2c_deinit(void);

#define BSP_SPIFFS_MOUNT_POINT      CONFIG_BSP_SPIFFS_MOUNT_POINT
esp_err_t bsp_spiffs_mount(void);
esp_err_t bsp_spiffs_unmount(void);

#define BSP_SD_MOUNT_POINT          CONFIG_BSP_SD_MOUNT_POINT
extern sdmmc_card_t *bsp_sdcard;
esp_err_t bsp_sdcard_mount(void);
esp_err_t bsp_sdcard_unmount(void);

lv_display_t *bsp_display_start(void);
//lv_display_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg);
lv_indev_t *bsp_display_get_input_dev(void);
bool bsp_display_lock(uint32_t timeout_ms);
void bsp_display_unlock(void);

void bsp_display_rotate(lv_display_t *disp, lv_display_rotation_t rotation);

#ifdef __cplusplus
}
#endif

