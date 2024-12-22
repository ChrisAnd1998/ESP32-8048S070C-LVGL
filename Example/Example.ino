// Requirements
// esp32 by Espressif Systems v3.0.7
// lvgl by kisvegabor v8.4.0
// TAMC_GT911 by TAMC v1.0.2
// GFX_Library_for_Arduino by Moon On Our Nation v1.5.0
// lv_conf.h file placed in your Arduino libraries directory

#include "DisplaySetup.h"

void btn_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

void lv_example_get_started_1() {
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(btn, 10, 10);
    lv_obj_set_size(btn, 120, 50);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);
}

void setup() {
    setupDisplayAndTouch();

    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello Arduino! (V8.4.0)");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_example_get_started_1();

    Serial.println("Setup complete");
}

void loop() {
    lv_timer_handler();
    delay(5);
}
