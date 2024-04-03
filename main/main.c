#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "bsp/esp-bsp.h"
#include "sdmmc_cmd.h"

static const char *TAG = "HMI";

extern void app_main_display();

void app_main(void)
{
    lv_display_t * disp;
    
    disp = bsp_display_start();
    bsp_display_rotate(disp, LV_DISP_ROTATION_270);

    ESP_LOGI(TAG, "Display LVGL UI");

    bsp_display_off();
    bsp_display_backlight_off();    /* Backlight to 0 */
    bsp_display_lock(0);            /* Lock exclusive */
    app_main_display();             /* Show LVGL objects */
    bsp_display_unlock();           /* Unlock */
    
    bsp_display_backlight_on();     /* Backlight to 100 */
    //bsp_display_brightness_set(80); /* Set display brightness percent */
    bsp_display_on();
    
    // Mount uSD card for testing
    if (ESP_OK == bsp_sdcard_mount()) {
        sdmmc_card_print_info(stdout, bsp_sdcard);
        FILE *f = fopen(BSP_SD_MOUNT_POINT "/hello.txt", "w");
        fprintf(f, "Hello %s!\n", bsp_sdcard->cid.name);
        fclose(f);
        bsp_sdcard_unmount();
    }
}
