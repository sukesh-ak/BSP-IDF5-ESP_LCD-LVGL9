#include "lvgl.h"
#include "esp_lvgl_port.h"

static void _app_button_cb(lv_event_t *e)
{
    lv_display_rotation_t rotation = (lv_display_rotation_t)lv_display_get_rotation(lv_display_get_default());
    rotation++;
    if (rotation > LV_DISPLAY_ROTATION_270) {
        rotation = LV_DISPLAY_ROTATION_0;
    }

    /* LCD HW rotation */
    lv_display_set_rotation(lv_display_get_default(), rotation);
}

void app_main_display()
{
    lv_obj_t *scr = lv_screen_active();

    /* Task lock */
    lvgl_port_lock(0);

    /* Your LVGL objects code here .... */

    /* Label */
    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_width(label, lv_obj_get_width(scr));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(label, LV_SYMBOL_BELL" Hello world Espressif and LVGL v9"LV_SYMBOL_BELL"\n "LV_SYMBOL_ENVELOPE" Using esp_lvgl_port from ESP-BSP");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -30);

    /* Button */
    lv_obj_t *btn = lv_btn_create(scr);
    label = lv_label_create(btn);
    lv_label_set_text_static(label, "Rotate screen");
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_add_event_cb(btn, _app_button_cb, LV_EVENT_CLICKED, NULL);

    /* Task unlock */
    lvgl_port_unlock();
}