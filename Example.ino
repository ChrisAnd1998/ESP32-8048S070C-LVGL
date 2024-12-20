//ESP32-S3-BOX 8048S070C - 7 inch TFT-LCD Display 800*480 pixels with Touchscreen Espressif 2.0.17

#include <lvgl.h> /* 8.4.0 */
#include <Arduino_GFX_Library.h> /* 1.3.1 */
#include "TAMC_GT911.h" /* 1.0.2 */

// Setup display
Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    GFX_NOT_DEFINED /* CS */, GFX_NOT_DEFINED /* SCK */, GFX_NOT_DEFINED /* SDA */,
    41 /* DE */, 40 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
    14 /* R0 */, 21 /* R1 */, 47 /* R2 */, 48 /* R3 */, 45 /* R4 */,
    9 /* G0 */, 46 /* G1 */, 3 /* G2 */, 8 /* G3 */, 16 /* G4 */, 1 /* G5 */,
    15 /* B0 */, 7 /* B1 */, 6 /* B2 */, 5 /* B3 */, 4 /* B4 */
);

Arduino_RPi_DPI_RGBPanel *gfx = new Arduino_RPi_DPI_RGBPanel(
    bus,
    800 /* width */, 0 /* hsync_polarity */, 210 /* hsync_front_porch */, 30 /* hsync_pulse_width */, 16 /* hsync_back_porch */,
    480 /* height */, 0 /* vsync_polarity */, 22 /* vsync_front_porch */, 13 /* vsync_pulse_width */, 10 /* vsync_back_porch */,
    1 /* pclk_active_neg */, 16000000 /* prefer_speed */, true /* auto_flush */
);

#define GFX_BL DF_GFX_BL // default backlight pin, you may replace DF_GFX_BL to actual backlight pin
#define TFT_BL 2

#define TOUCH_GT911
#define TOUCH_GT911_SCL 20
#define TOUCH_GT911_SDA 19
#define TOUCH_GT911_INT -1
#define TOUCH_GT911_RST 38
#define TOUCH_GT911_ROTATION ROTATION_NORMAL
#define TOUCH_MAP_X1 800
#define TOUCH_MAP_X2 0
#define TOUCH_MAP_Y1 480
#define TOUCH_MAP_Y2 0

TAMC_GT911 ts = TAMC_GT911(TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST, max(TOUCH_MAP_X1, TOUCH_MAP_X2), max(TOUCH_MAP_Y1, TOUCH_MAP_Y2));

static uint32_t screenWidth;
static uint32_t screenHeight;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;
static lv_color_t *disp_draw_buf2;
static lv_disp_drv_t disp_drv;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

    lv_disp_flush_ready(disp);
}

/* Read touch input and pass it to LVGL */
void my_touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    ts.read(); // Read touch data from TAMC_GT911
    if (ts.isTouched) {
        data->state = LV_INDEV_STATE_PR; // Pressed
        data->point.x = ts.points[0].x; // Get X coordinate
        data->point.y = ts.points[0].y; // Get Y coordinate
    } else {
        data->state = LV_INDEV_STATE_REL; // Released
    }
}

/* Button event callback */
static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        /* Get the first child of the button which is the label and change its text */
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

/* Create a button with a label and react on click event */
void lv_example_get_started_1(void)
{
    /* Create button */
    lv_obj_t * btn = lv_btn_create(lv_scr_act());     /* Add a button to the current screen */
    lv_obj_set_pos(btn, 10, 10);                      /* Set its position */
    lv_obj_set_size(btn, 120, 50);                    /* Set its size */
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL); /* Assign a callback to the button */

    lv_obj_t * label = lv_label_create(btn);          /* Add a label to the button */
    lv_label_set_text(label, "Button");               /* Set the label's text */
    lv_obj_center(label);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("LVGL Touch Button Example");

#ifdef GFX_PWD
    pinMode(GFX_PWD, OUTPUT);
    digitalWrite(GFX_PWD, HIGH);
#endif

    // Init Display
    gfx->begin();
    gfx->fillScreen(BLACK);

#ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
#endif

    lv_init();

    screenWidth = gfx->width();
    screenHeight = gfx->height();

#ifdef ESP32
    disp_draw_buf = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * 32, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#else
    disp_draw_buf = (lv_color_t *)malloc(sizeof(lv_color_t) * screenWidth * 32);
#endif

    if (!disp_draw_buf) {
        Serial.println("LVGL disp_draw_buf allocation failed!");
        return;
    }

    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * 32);

    /* Initialize the display */
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /* Initialize the touch screen */
    ts.begin();
    ts.setRotation(1);

    /* Register the touch input device with LVGL */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER; // Touchscreen as pointer input
    indev_drv.read_cb = my_touchpad_read;   // Use custom touchpad read function
    lv_indev_drv_register(&indev_drv);

    /* Create simple label */
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello Arduino! (V8.4.0)");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    /* Create the button example */
    lv_example_get_started_1();

    Serial.println("Setup complete");
}

void loop()
{
    lv_timer_handler(); /* Let the GUI do its work */
    delay(5);
}
