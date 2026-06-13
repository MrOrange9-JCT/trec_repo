#include <lvgl.h>
#include <epd_driver.h>
#include "ui.h"

#define EPD_WIDTH  960
#define EPD_HEIGHT 540

static lv_color_t *lvgl_buf;
static uint8_t    *epd_fb;

// ── LVGL → EPDiy flush ──────────────────────────────────────────
void epd_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            lv_color_t c = *color_p++;
            uint8_t gray = (c.ch.red * 77 + c.ch.green * 150 + c.ch.blue * 29) >> 8;
            epd_draw_pixel(x, y, gray, epd_fb);
        }
    }
    if (lv_disp_flush_is_last(drv)) {
        epd_poweron();
        epd_clear();
        epd_draw_grayscale_image(epd_full_screen(), epd_fb);
        epd_poweroff();
    }
    lv_disp_flush_ready(drv);
}

// ── Setup ───────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);

    // EPDiy
    epd_init();
    epd_fb = (uint8_t *)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT / 2, MALLOC_CAP_SPIRAM);
    memset(epd_fb, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

    // LVGL
    lv_init();
    lvgl_buf = (lv_color_t *)heap_caps_malloc(EPD_WIDTH * 40 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);

    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, lvgl_buf, NULL, EPD_WIDTH * 40);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res  = EPD_WIDTH;
    disp_drv.ver_res  = EPD_HEIGHT;
    disp_drv.flush_cb = epd_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // EEZ Studio — one call does everything
    ui_init();
}

// ── Loop ────────────────────────────────────────────────────────
void loop() {
    lv_timer_handler();
    ui_tick();
    delay(5);
}