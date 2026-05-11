#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <SPI.h>
#include "lvcfg.h"
#include <lvgl.h>
#include "maps.h"
#include "input.h"

#define W 320
#define H 170

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[2][W * 10]; // what.

static bool simple_rendering = false;

void etft() {
    digitalWrite(13, LOW);
    digitalWrite(5, HIGH);
}
void esd() {
    digitalWrite(13, HIGH);
    digitalWrite(5, LOW);
}

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI _bus_instance;

public:
    // https://garrysblog.com/2025/11/10/lovyangfx-display-config-code-ili9341-gc9a01-and-st7789-displays/
    LGFX() {
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = 1;
            cfg.spi_mode = 0;
            cfg.freq_write = 640000000; // how does this work???
            cfg.freq_read = 10000000;
            cfg.pin_sclk = 14;
            cfg.pin_mosi = 15;
            cfg.pin_miso = 12;
            cfg.pin_dc = 27;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = 13;
            cfg.pin_rst = 29;
            cfg.pin_busy = -1;
            cfg.panel_width = H; // swapped bc display is rotated
            cfg.panel_height = W;
            cfg.offset_x = 34;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = false;
            cfg.invert = true;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = false;
            _panel_instance.config(cfg);
        }

        setPanel(&_panel_instance);
    }
};

LGFX tft = LGFX();

// https://github.com/lovyan03/LovyanGFX/blob/master/examples/Advanced/LVGL_PlatformIO/src/main.cpp#L17
void flush_gfx(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
    if(tft.getStartCount() == 0)
        tft.startWrite();
    // hah. screw you google. screw you chatgpt. rgb565_t instead of swap565_t was the fix. and not even uint16_t. not even anything in lvcfg.h. hahah.
    tft.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::rgb565_t*)&color_p->full);
    tft.endWrite();
    lv_disp_flush_ready(disp);
}

Level lvl(""); // dummy level, we'll load the actual thing later on
lv_obj_t* score;
lv_obj_t* combo;
lv_obj_t* rating;
lv_obj_t* delta;
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("init");

    init_input();

    etft();
    tft.init();
    tft.setRotation(3);
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf[0], buf[1], W * 10);

    // https://github.com/lovyan03/LovyanGFX/blob/master/examples/Advanced/LVGL_PlatformIO/src/main.cpp#L67
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = W;
    disp_drv.ver_res = H;
    disp_drv.flush_cb = flush_gfx;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    esd();
    maps_init();

    auto levels = maps_list();
    auto diffs = difficulty_list(levels[4]);
    Serial.println(diffs[1]);
    lvl = load_level(diffs[1]);
    etft();

    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);

    score = lv_label_create(lv_scr_act());
    lv_obj_set_x(score, 10);
    lv_obj_set_y(score, 0);
    lv_obj_set_style_text_color(score, lv_color_white(), 0);
    lv_label_set_text(score, "0");

    combo = lv_label_create(lv_scr_act());
    lv_obj_set_x(combo, 250);
    lv_obj_set_y(combo, 0);
    lv_obj_set_style_text_color(combo, lv_color_white(), 0);
    lv_label_set_text(combo, "0");

    rating = lv_label_create(lv_scr_act());
    lv_obj_set_x(rating, 15);
    lv_obj_set_y(rating, 17);
    lv_obj_set_style_text_align(rating, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(rating, lv_color_white(), 0);
    lv_obj_set_width(rating, 50);
    lv_label_set_text(rating, "");

    delta = lv_label_create(lv_scr_act());
    lv_obj_set_x(delta, 15);
    lv_obj_set_y(delta, 32);
    lv_obj_set_style_text_align(delta, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(delta, lv_color_white(), 0);
    lv_obj_set_width(delta, 50);
    lv_label_set_text(delta, "");

    for(int i = 35; i >= 20; i -= 15) {
        lv_obj_t* circle = lv_obj_create(lv_scr_act());
        lv_obj_set_x(circle, 40 - i);
        lv_obj_set_y(circle, 170 / 2 - i);
        lv_obj_set_size(circle, 2 * i, 2 * i);
        lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(circle, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(circle, lv_palette_main(LV_PALETTE_GREY), 0);
        lv_obj_set_style_border_width(circle, 2, 0);
    }
}
std::vector<lv_obj_t*> past;
uint8_t last_mask = 0;
void loop() {
    uint8_t mask = get_input();
    uint8_t pressed = (mask ^ last_mask) & mask;
    if(check_input(pressed, INPUT_DON_RIGHT)) Serial.println("DON_RIGHT");
    if(check_input(pressed, INPUT_DON_LEFT)) Serial.println("DON_LEFT");
    if(check_input(pressed, INPUT_KAT_RIGHT)) Serial.println("KAT_RIGHT");
    if(check_input(pressed, INPUT_KAT_LEFT)) Serial.println("KAT_LEFT");
    if(check_input(pressed, INPUT_BOOTSEL)) Serial.println("BOOTSEL");
    lvl.btn(millis(), pressed);
    last_mask = mask;

    for(auto x : past)
        lv_obj_del(x);
    past.clear();
    auto cur = lvl.render(millis());
    for(auto x : cur) {
        int r = x.big ? 35 : 20;
        auto c = x.type & 8 ? LV_PALETTE_INDIGO : x.type & 2 ? LV_PALETTE_YELLOW : x.kat ? LV_PALETTE_CYAN : LV_PALETTE_RED;

        lv_obj_t* circle = lv_obj_create(lv_scr_act());
        lv_obj_set_x(circle, x.x - r);
        lv_obj_set_y(circle, 170 / 2 - r);
        lv_obj_set_size(circle, 2 * r + x.len, 2 * r);
        lv_obj_set_style_radius(circle, r, 0);
        lv_obj_set_style_bg_color(circle, lv_palette_main(c), 0);
        lv_obj_set_style_bg_opa(circle, simple_rendering ? LV_OPA_TRANSP : LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(circle, simple_rendering ? 2 : 0, 0);
        lv_obj_set_style_border_color(circle, lv_palette_main(c), 0);
        past.push_back(circle);
    }
    
    auto rat = lvl.get_rating(millis());
    lv_label_set_text(rating, rat.txt.c_str());
    lv_label_set_text(delta, rat.delta.c_str());
    lv_obj_set_style_text_opa(rating, rat.opacity * 255, 0);
    lv_obj_set_style_text_opa(delta, rat.opacity * 255, 0);

    lv_label_set_text(score, String(lvl.score).c_str());
    lv_label_set_text(combo, String(lvl.combo).c_str());

    lv_refr_now(lv_disp_get_default());
    lv_timer_handler();
}