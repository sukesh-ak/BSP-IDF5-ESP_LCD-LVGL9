/**
 * @file
 * @brief BSP LCD
 *
 * This file offers API for basic LCD control.
 * It is useful for users who want to use the LCD without the default Graphical Library LVGL.
 *
 * For standard LCD initialization with LVGL graphical library, you can call all-in-one function bsp_display_start().
 */

#pragma once
#include "esp_lcd_types.h"

/* LCD color formats */
#define ESP_LCD_COLOR_FORMAT_RGB565    (1)
#define ESP_LCD_COLOR_FORMAT_RGB888    (2)

/* LCD display color format */
#define BSP_LCD_COLOR_FORMAT        (ESP_LCD_COLOR_FORMAT_RGB565)

/* LCD display color bytes endianess */
#define BSP_LCD_BIGENDIAN           (1)

/* LCD display color bits */
#define BSP_LCD_BITS_PER_PIXEL      (16)

/* LCD display color space */
#define BSP_LCD_COLOR_SPACE         (ESP_LCD_COLOR_SPACE_BGR)

/* LCD definition */
#define BSP_LCD_H_RES              (320)
#define BSP_LCD_V_RES              (480)

#define BSP_LCD_PIXEL_CLOCK_HZ     (20 * 1000 * 1000)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BSP display configuration structure
 *
 */
typedef struct {
    int max_transfer_sz;    /*!< Maximum transfer size, in bytes. */
} bsp_display_config_t;

esp_err_t bsp_display_new(const bsp_display_config_t *config, esp_lcd_panel_handle_t *ret_panel, esp_lcd_panel_io_handle_t *ret_io);
esp_err_t bsp_display_brightness_set(int brightness_percent);
esp_err_t bsp_display_backlight_on(void);
esp_err_t bsp_display_backlight_off(void);

#ifdef __cplusplus
}
#endif

