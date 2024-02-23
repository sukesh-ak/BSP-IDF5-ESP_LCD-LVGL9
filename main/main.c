
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

#if CONFIG_HMI_LCD_CONTROLLER_ST7796
#include "esp_lcd_st7796.h"
#endif

/* Check if its required for sleep */
// #if CONFIG_HMI_LCD_TOUCH_ENABLED
// #include <esp_lcd_touch.h>
// #endif

#if CONFIG_HMI_LCD_TOUCH_CONTROLLER_FT5X06
#include "esp_lcd_touch_ft5x06.h"
#endif
#include <driver/i2c.h>


static const char *TAG = "HMI";

// Using SPI2 in the example
#define LCD_HOST  SPI2_HOST

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define HMI_LCD_PIXEL_CLOCK_HZ     (20 * 1000 * 1000)
#define HMI_LCD_BK_LIGHT_ON_LEVEL  1
#define HMI_LCD_BK_LIGHT_OFF_LEVEL !HMI_LCD_BK_LIGHT_ON_LEVEL

#define HMI_PIN_NUM_SCLK           14
#define HMI_PIN_NUM_MOSI           13
#define HMI_PIN_NUM_MISO           -1
#define HMI_PIN_NUM_LCD_DC         21
#define HMI_PIN_NUM_LCD_RST        22
#define HMI_PIN_NUM_LCD_CS         15

#define HMI_PIN_NUM_BK_LIGHT       23
#define HMI_PIN_NUM_TOUCH_CS       15   // ??

#define HMI_TOUCH_I2C_NUM 1
#define HMI_I2C_CLK_SPEED_HZ 400000
#define HMI_PIN_TOUCH_INT   GPIO_NUM_39
#define HMI_PIN_TOUCH_SDA   GPIO_NUM_18
#define HMI_PIN_TOUCH_SCL   GPIO_NUM_19

// The pixel number in horizontal and vertical
#if CONFIG_HMI_LCD_CONTROLLER_ST7796
#define HMI_LCD_H_RES              320
#define HMI_LCD_V_RES              480
#endif

// Bit number used to represent command and parameter
#define HMI_LCD_CMD_BITS           8
#define HMI_LCD_PARAM_BITS         8

// LVGL Settings
#define HMI_LVGL_TICK_PERIOD_MS    2
#define HMI_LVGL_TASK_MAX_DELAY_MS 500
#define HMI_LVGL_TASK_MIN_DELAY_MS 1
#define HMI_LVGL_TASK_STACK_SIZE   (4 * 1024)
#define HMI_LVGL_TASK_PRIORITY     2

#define HMI_LVGL_TICK_PERIOD_MS    2

static SemaphoreHandle_t lvgl_mux = NULL;

#if CONFIG_HMI_LCD_TOUCH_ENABLED
esp_lcd_touch_handle_t tp = NULL;
#endif

extern void hmi_lvgl_demo_ui(lv_disp_t *disp);

// Lock using mutex for multi-threading/tasking support
bool hmi_lvgl_lock(int timeout_ms)
{
    // Convert timeout in milliseconds to FreeRTOS ticks
    // If `timeout_ms` is set to -1, the program will block until the condition is met
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvgl_mux, timeout_ticks) == pdTRUE;
}

void hmi_lvgl_unlock(void)
{
    xSemaphoreGiveRecursive(lvgl_mux);
}

static void hmi_lvgl_task(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t task_delay_ms = HMI_LVGL_TASK_MAX_DELAY_MS;
    while (1) {
        // Lock the mutex since LVGL APIs are not thread-safe
        if (hmi_lvgl_lock(-1)) {
            task_delay_ms = lv_timer_handler();
            // Release the mutex
            hmi_lvgl_unlock();
        }
        if (task_delay_ms > HMI_LVGL_TASK_MAX_DELAY_MS) {
            task_delay_ms = HMI_LVGL_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < HMI_LVGL_TASK_MIN_DELAY_MS) {
            task_delay_ms = HMI_LVGL_TASK_MIN_DELAY_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

static bool hmi_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void hmi_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

/* Rotate display and touch, when rotated screen in LVGL. Called when driver parameters are updated. */
static void hmi_lvgl_port_update_callback(lv_disp_drv_t *drv)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;

    switch (drv->rotated) {
    case LV_DISP_ROT_NONE:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, false);
        esp_lcd_panel_mirror(panel_handle, true, false);
#if CONFIG_HMI_LCD_TOUCH_ENABLED
        // Rotate LCD touch
        esp_lcd_touch_set_mirror_y(tp, false);
        esp_lcd_touch_set_mirror_x(tp, false);
#endif
        break;
    case LV_DISP_ROT_90:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, true);
        esp_lcd_panel_mirror(panel_handle, true, true);
#if CONFIG_HMI_LCD_TOUCH_ENABLED
        // Rotate LCD touch
        esp_lcd_touch_set_mirror_y(tp, false);
        esp_lcd_touch_set_mirror_x(tp, false);
#endif
        break;
    case LV_DISP_ROT_180:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, false);
        esp_lcd_panel_mirror(panel_handle, false, true);
#if CONFIG_HMI_LCD_TOUCH_ENABLED
        // Rotate LCD touch
        esp_lcd_touch_set_mirror_y(tp, false);
        esp_lcd_touch_set_mirror_x(tp, false);
#endif
        break;
    case LV_DISP_ROT_270:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, true);
        esp_lcd_panel_mirror(panel_handle, false, false);
#if CONFIG_HMI_LCD_TOUCH_ENABLED
        // Rotate LCD touch
        esp_lcd_touch_set_mirror_y(tp, false);
        esp_lcd_touch_set_mirror_x(tp, false);
#endif
        break;
    }
}

#if CONFIG_HMI_LCD_TOUCH_ENABLED
static void hmi_lvgl_touch_cb(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    /* Read touch controller data */
    esp_lcd_touch_read_data(drv->user_data);

    /* Get coordinates */
    bool touchpad_pressed = esp_lcd_touch_get_coordinates(drv->user_data, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

    if (touchpad_pressed && touchpad_cnt > 0) {
        data->point.x = touchpad_x[0];
        data->point.y = touchpad_y[0];
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
#endif

static void hmi_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(HMI_LVGL_TICK_PERIOD_MS);
}

//extern "C" 
void app_main(void)
{
    static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
    static lv_disp_drv_t disp_drv;      // contains callback functions

    ESP_LOGI(TAG, "Turn off LCD backlight");
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << HMI_PIN_NUM_BK_LIGHT
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num = HMI_PIN_NUM_SCLK,
        .mosi_io_num = HMI_PIN_NUM_MOSI,
        .miso_io_num = HMI_PIN_NUM_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = HMI_LCD_H_RES * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

#if CONFIG_HMI_LCD_CONTROLLER_ST7796
    ESP_LOGI(TAG, "Initialize I2C bus for Touch");
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
#endif

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = HMI_PIN_NUM_LCD_DC,
        .cs_gpio_num = HMI_PIN_NUM_LCD_CS,
        .pclk_hz = HMI_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = HMI_LCD_CMD_BITS,
        .lcd_param_bits = HMI_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = hmi_notify_lvgl_flush_ready,
        .user_ctx = &disp_drv,
    };
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = HMI_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
#if CONFIG_HMI_LCD_CONTROLLER_ST7796
    ESP_LOGI(TAG, "Install ST7796 panel driver");
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_config, &panel_handle));
#endif

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));

    // user can flush pre-defined pattern to the screen before we turn on the screen or backlight
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

#if CONFIG_HMI_LCD_TOUCH_ENABLED
    esp_lcd_touch_handle_t tp_io_handle = NULL;
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = HMI_LCD_H_RES,
        .y_max = HMI_LCD_V_RES,
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
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

    #if CONFIG_HMI_LCD_TOUCH_CONTROLLER_FT5X06
        esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
        // Attach the TOUCH to the I2C bus
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)HMI_TOUCH_I2C_NUM, &tp_io_config, &tp_io_handle));
        ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &tp));    
    #endif // CONFIG_HMI_LCD_TOUCH_CONTROLLER_FT5X06
#endif // CONFIG_HMI_LCD_TOUCH_ENABLED

    ESP_LOGI(TAG, "Turn on LCD backlight");
    gpio_set_level(HMI_PIN_NUM_BK_LIGHT, HMI_LCD_BK_LIGHT_ON_LEVEL);

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    // alloc draw buffers used by LVGL
    // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
    lv_color_t *buf1 = heap_caps_malloc(HMI_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1);
    lv_color_t *buf2 = heap_caps_malloc(HMI_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);

    // initialize LVGL draw buffers
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, HMI_LCD_H_RES * 20);
    
    // TODO: LVGL v9
    //lv_display_set_buffers

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = HMI_LCD_H_RES;
    disp_drv.ver_res = HMI_LCD_V_RES;
    disp_drv.flush_cb = hmi_lvgl_flush_cb;
    disp_drv.drv_update_cb = hmi_lvgl_port_update_callback;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &hmi_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, HMI_LVGL_TICK_PERIOD_MS * 1000));

#if CONFIG_HMI_LCD_TOUCH_ENABLED
    static lv_indev_drv_t indev_drv;    // Input device driver (Touch)
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.disp = disp;
    indev_drv.read_cb = hmi_lvgl_touch_cb;
    indev_drv.user_data = tp;

    lv_indev_drv_register(&indev_drv);
#endif

    // To support Multi-threading/tasking for LVGL
    lvgl_mux = xSemaphoreCreateRecursiveMutex();
    assert(lvgl_mux);

    ESP_LOGI(TAG, "Create LVGL task");
    // Pin it to 2nd core later
    xTaskCreate(hmi_lvgl_task, "LVGL", HMI_LVGL_TASK_STACK_SIZE, NULL, HMI_LVGL_TASK_PRIORITY, NULL);

    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (hmi_lvgl_lock(-1)) {
        
        ESP_LOGI(TAG, "Display LVGL Meter Widget");
        hmi_lvgl_demo_ui(disp);

        // Release the mutex
        hmi_lvgl_unlock();
    }
}
