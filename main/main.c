#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lv_demos.h"
#include "bsp/esp-bsp.h"

static const char *TAG = "HMI";

extern void app_main_display();

void app_main(void)
{
    lv_display_t * disp;
    
    disp = bsp_display_start();
    bsp_display_rotate(disp, LV_DISP_ROTATION_270);

    ESP_LOGI(TAG, "Display LVGL UI");

    bsp_display_backlight_off();

    bsp_display_lock(0);    /* Lock exclusive */
    app_main_display();     /* Show LVGL objects */
    bsp_display_unlock();   /* Unlock */

    bsp_display_brightness_set(80); /* Set display brightness percent */

    bsp_display_backlight_on();
}
