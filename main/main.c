
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"

#if CONFIG_HMI_LCD_CONTROLLER_ST7796 || CONFIG_HMI_LCD_CONTROLLER_ST7796P8
#include "esp_lcd_st7796.h"
#endif

#if CONFIG_HMI_LCD_TOUCH_CONTROLLER_FT5X06
#include "esp_lcd_touch_ft5x06.h"
#endif
#include <driver/i2c.h>

static const char *TAG = "HMI";



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define HMI_LCD_PIXEL_CLOCK_HZ     (20 * 1000 * 1000)
#define HMI_LCD_BK_LIGHT_ON_LEVEL  1
#define HMI_LCD_BK_LIGHT_OFF_LEVEL !HMI_LCD_BK_LIGHT_ON_LEVEL

#if CONFIG_HMI_LCD_CONTROLLER_ST7796
    // WT32-SC01 - ST7789 SPI

    // Using SPI2 in the example
    #define LCD_HOST  SPI2_HOST

    #define HMI_PIN_NUM_SCLK           14
    #define HMI_PIN_NUM_MOSI           13
    #define HMI_PIN_NUM_MISO           -1
    #define HMI_PIN_NUM_LCD_DC         21
    #define HMI_PIN_NUM_LCD_RST        22
    #define HMI_PIN_NUM_LCD_CS         15

    #define HMI_PIN_NUM_BK_LIGHT       23

#elif CONFIG_HMI_LCD_CONTROLLER_ST7796P8 // WORK IN PROGRESS

    // WT32-SC01 Plus - ST7789 8 Bit parellel
    #define HMI_PIN_NUM_DATA0          9
    #define HMI_PIN_NUM_DATA1          46
    #define HMI_PIN_NUM_DATA2          3
    #define HMI_PIN_NUM_DATA3          8
    #define HMI_PIN_NUM_DATA4          18
    #define HMI_PIN_NUM_DATA5          17

    #define HMI_PIN_NUM_DATA6          16
    #define HMI_PIN_NUM_DATA7          15

    #define HMI_PIN_NUM_CS            -1
    #define HMI_PIN_NUM_RESET          4
    #define HMI_PIN_NUM_WR             47
    #define HMI_PIN_NUM_RS             0
    #define HMI_PIN_NUM_DC             0
    #define HMI_PIN_NUM_BK_LIGHT       45

    #define HMI_PIN_NUM_DATA8      (-1)
    #define HMI_PIN_NUM_DATA9      (-1)
    #define HMI_PIN_NUM_DATA10     (-1)
    #define HMI_PIN_NUM_DATA11     (-1)
    #define HMI_PIN_NUM_DATA12     (-1)
    #define HMI_PIN_NUM_DATA13     (-1)
    #define HMI_PIN_NUM_DATA14     (-1)
    #define HMI_PIN_NUM_DATA15     (-1)    
#endif

// Touch PINS
#define HMI_TOUCH_I2C_NUM I2C_NUM_1
#define HMI_I2C_CLK_SPEED_HZ 400000

#if CONFIG_HMI_LCD_CONTROLLER_ST7796
    #define HMI_PIN_TOUCH_INT   GPIO_NUM_39
    #define HMI_PIN_TOUCH_SDA   GPIO_NUM_18
    #define HMI_PIN_TOUCH_SCL   GPIO_NUM_19
#elif CONFIG_HMI_LCD_CONTROLLER_ST7796P8
    #define HMI_PIN_TOUCH_INT   GPIO_NUM_7
    #define HMI_PIN_TOUCH_SDA   GPIO_NUM_6
    #define HMI_PIN_TOUCH_SCL   GPIO_NUM_5
#endif

// The pixel number in horizontal and vertical
#if CONFIG_HMI_LCD_CONTROLLER_ST7796 || CONFIG_HMI_LCD_CONTROLLER_ST7796P8
#define HMI_LCD_H_RES              320
#define HMI_LCD_V_RES              480
#define HMI_LCD_DRAW_BUFF_HEIGHT (50)
#define HMI_LCD_COLOR_SPACE     (ESP_LCD_COLOR_SPACE_BGR)
#define HMI_LCD_BITS_PER_PIXEL  (16)
#define HMI_LCD_DRAW_BUFF_DOUBLE (1)

#define HMI_LCD_BUS_WIDTH   (8)   // data width
#endif

// Bit number used to represent command and parameter
#define HMI_LCD_CMD_BITS           8
#define HMI_LCD_PARAM_BITS         8

// LVGL Settings
#define HMI_LVGL_TICK_PERIOD_MS    2
#define HMI_LVGL_TASK_MAX_DELAY_MS 500
#define HMI_LVGL_TASK_MIN_DELAY_MS 5
#define HMI_LVGL_TASK_STACK_SIZE   (4 * 1024)
#define HMI_LVGL_TASK_PRIORITY     4

extern void app_main_display();

/* LCD IO and panel */
static esp_lcd_panel_io_handle_t lcd_panel_io_handle = NULL;
static esp_lcd_panel_handle_t lcd_panel_handle = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;

/* LVGL display and touch */
static lv_display_t *lvgl_disp = NULL;
static lv_indev_t *lvgl_touch_indev = NULL;


#if CONFIG_HMI_LCD_CONTROLLER_ST7796
static esp_err_t app_lcd_spi_init(void)
{
    esp_err_t ret = ESP_OK;

    /* LCD backlight */
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << HMI_PIN_NUM_BK_LIGHT
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    /* LCD initialization */
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = HMI_PIN_NUM_SCLK,
        .mosi_io_num = HMI_PIN_NUM_MOSI,
        .miso_io_num = HMI_PIN_NUM_MISO,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = HMI_LCD_H_RES * HMI_LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = HMI_PIN_NUM_LCD_DC,
        .cs_gpio_num = HMI_PIN_NUM_LCD_CS,
        .pclk_hz = HMI_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = HMI_LCD_CMD_BITS,
        .lcd_param_bits = HMI_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &lcd_panel_io_handle));

    ESP_LOGD(TAG, "Install LCD driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = HMI_PIN_NUM_LCD_RST,
        .color_space = HMI_LCD_COLOR_SPACE,
        .bits_per_pixel = HMI_LCD_BITS_PER_PIXEL,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(lcd_panel_io_handle, &panel_config, &lcd_panel_handle));

    esp_lcd_panel_reset(lcd_panel_handle);
    esp_lcd_panel_init(lcd_panel_handle);

    esp_lcd_panel_mirror(lcd_panel_handle, false, true);
   
    esp_lcd_panel_disp_on_off(lcd_panel_handle, true);

    /* LCD backlight on */
    ESP_ERROR_CHECK(gpio_set_level(HMI_PIN_NUM_BK_LIGHT, HMI_LCD_BK_LIGHT_ON_LEVEL));
    return ret;
}
#elif CONFIG_HMI_LCD_CONTROLLER_ST7796P8
static esp_err_t app_lcd_i80_init(void)
{
    esp_err_t ret = ESP_OK;
    /* LCD backlight */
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << HMI_PIN_NUM_BK_LIGHT
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    /* LCD initialization */
    ESP_LOGI(TAG, "Initialize Intel 8080 bus");
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    //HMI_LCD_H_RES * HMI_LCD_V_RES * HMI_LCD_BITS_PER_PIXEL / 8, 
    esp_lcd_i80_bus_config_t bus_config = ST7796_PANEL_BUS_I80_CONFIG(
            HMI_LCD_H_RES * HMI_LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t),
            HMI_LCD_BUS_WIDTH,
            HMI_PIN_NUM_DC, 
            HMI_PIN_NUM_WR,
            HMI_PIN_NUM_DATA0, 
            HMI_PIN_NUM_DATA1, 
            HMI_PIN_NUM_DATA2, 
            HMI_PIN_NUM_DATA3,
            HMI_PIN_NUM_DATA4, 
            HMI_PIN_NUM_DATA5, 
            HMI_PIN_NUM_DATA6, 
            HMI_PIN_NUM_DATA7,

            HMI_PIN_NUM_DATA8, 
            HMI_PIN_NUM_DATA9, 
            HMI_PIN_NUM_DATA10, 
            HMI_PIN_NUM_DATA11,
            HMI_PIN_NUM_DATA12, 
            HMI_PIN_NUM_DATA13, 
            HMI_PIN_NUM_DATA14, 
            HMI_PIN_NUM_DATA15);

    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = HMI_PIN_NUM_CS,
        .pclk_hz = HMI_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
    };

        //ST7796_PANEL_IO_I80_CONFIG(-1, example_callback, &example_callback_ctx);
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &lcd_panel_io_handle));

    ESP_LOGI(TAG, "Install ST7796 panel driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = HMI_PIN_NUM_RESET,      // Set to -1 if not use
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = HMI_LCD_BITS_PER_PIXEL,    // Implemented by LCD command `3Ah` (16/18/24)
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(lcd_panel_io_handle, &panel_config, &lcd_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_panel_handle, true));
    return ret;
}
#endif

// Touch FT5x06
static esp_err_t app_touch_init(void)
{
    /* Initilize I2C */
    const i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = HMI_PIN_TOUCH_SDA,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_io_num = HMI_PIN_TOUCH_SCL,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = HMI_I2C_CLK_SPEED_HZ
    };
    ESP_ERROR_CHECK(i2c_param_config(HMI_TOUCH_I2C_NUM, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(HMI_TOUCH_I2C_NUM, i2c_conf.mode, 0, 0, 0));

    /* Initialize touch HW */
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = HMI_LCD_H_RES,
        .y_max = HMI_LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_NC, // Shared with LCD reset
        .int_gpio_num = HMI_PIN_TOUCH_INT,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    const esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)HMI_TOUCH_I2C_NUM, &tp_io_config, &tp_io_handle));

    return esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &touch_handle);
}

static esp_err_t app_lvgl_init(void)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = HMI_LVGL_TASK_PRIORITY,            /* LVGL task priority */
        .task_stack = HMI_LVGL_TASK_STACK_SIZE,             /* LVGL task stack size */
        .task_affinity = 1,                                /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = HMI_LVGL_TASK_MAX_DELAY_MS,    /* Maximum sleep in LVGL task */
        .timer_period_ms = HMI_LVGL_TASK_MIN_DELAY_MS       /* LVGL timer tick period in ms */
    };
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_panel_io_handle,
        .panel_handle = lcd_panel_handle,
        .buffer_size = HMI_LCD_H_RES * HMI_LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t),
        .double_buffer = HMI_LCD_DRAW_BUFF_DOUBLE,
        .hres = HMI_LCD_H_RES,
        .vres = HMI_LCD_V_RES,
        .monochrome = false,
        /* Rotation values must be same as used in esp_lcd for initial settings of the screen */
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = true,
        },
        .flags = {
            .buff_dma = false,
            .buff_spiram = true,
            .swap_bytes = true,
        }
    };
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);

    /* Add touch input (for selected screen) */
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = lvgl_disp,
        .handle = touch_handle,
    };
    lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg);

    // Added to fix the touch points
    esp_lcd_touch_set_mirror_y(touch_handle, true);
    esp_lcd_touch_set_mirror_x(touch_handle, true);

    return ESP_OK;
}

void app_main(void)
{
#if CONFIG_HMI_LCD_CONTROLLER_ST7796    
    /* WT32-SC01 - LCD HW initialization */
    ESP_ERROR_CHECK(app_lcd_spi_init());
#elif CONFIG_HMI_LCD_CONTROLLER_ST7796P8
    /* WT32-SC01 PLUS - LCD HW initialization */
    ESP_ERROR_CHECK(app_lcd_i80_init());
#endif

    /* Touch initialization */
    ESP_ERROR_CHECK(app_touch_init());

    /* LVGL initialization */
    ESP_ERROR_CHECK(app_lvgl_init());
    
    /* Show LVGL objects */
    app_main_display();
    
}