#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "others/observer/lv_observer.h"
#include "bsp/wt32sc01plus.h"

lv_subject_t brightness_subject;

LV_IMG_DECLARE(emoji);   

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

static void brightness_observer_cb(lv_observer_t * observer, lv_subject_t * subject)
{
    int32_t brightness_percent = lv_subject_get_int(subject);
    bsp_display_brightness_set(brightness_percent);
}

void app_main_display()
{
    lv_obj_t *scr = lv_screen_active();

    /* DEMO of using LVGL lv_observer features below */

    // Initialize lv_subject into int (int32_t) type
    lv_subject_init_int(&brightness_subject, 80);

    /*Create a slider in the center of the display*/
    lv_obj_t * slider = lv_slider_create(lv_screen_active());
    lv_obj_align(slider,LV_ALIGN_CENTER,0,10);
    lv_slider_bind_value(slider, &brightness_subject);                  // Bind slider value with the lv_subject
    lv_obj_set_style_anim_duration(slider, 2000, 0);

    /*Create a label below the slider*/
    lv_obj_t * slider_label = lv_label_create(lv_screen_active());
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_label_bind_text(slider_label, &brightness_subject, "%d %%");     // Bind label text with lv_subject

    // Add lv_observer with a callback when the value changes
    lv_subject_add_observer_obj(&brightness_subject, brightness_observer_cb, NULL, NULL);

    /* Emoji - image */
    lv_obj_t *img_emoji = lv_image_create(scr);
    lv_img_set_src(img_emoji,&emoji);
    lv_obj_align(img_emoji, LV_ALIGN_TOP_MID, 0, 10);

    /* Label */
    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_width(label, lv_obj_get_width(scr));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text_fmt(label, LV_SYMBOL_BELL" Hello world with LVGL v%d.%d.%d "LV_SYMBOL_BELL"\n "LV_SYMBOL_ENVELOPE" Using esp_lvgl_port from ESP-BSP",
                            lv_version_major(),lv_version_minor(),lv_version_patch());
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -30);

    /* Button */
    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_set_size(btn,LV_SIZE_CONTENT,50);
    
    label = lv_label_create(btn);
    lv_obj_align_to(label,btn,LV_ALIGN_CENTER,0,0);
    lv_label_set_text_static(label, "Rotate screen");
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -50);
    lv_obj_add_event_cb(btn, _app_button_cb, LV_EVENT_CLICKED, NULL);
}