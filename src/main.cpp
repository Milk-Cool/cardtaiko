#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <SPI.h>
#include "lvcfg.h"
#include <lvgl.h>
#include "maps.h"
#include "input.h"
#include "en.h"
#include "audio.h"
#include <NeoPixelConnect.h>
#include <Keyboard.h>

#define W 320
#define H 170

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[2][W * 10]; // what.

static bool simple_rendering = false;

static NeoPixelConnect p(16, 1);

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

static String filename(String path) {
    auto idx = path.lastIndexOf('/');
    if(idx == path.length() - 1) idx = path.substring(0, path.length() - 1).lastIndexOf('/');
    return path.substring(idx + 1);
}
static String diffname(String filename) {
    return filename.substring(filename.indexOf('[') + 1, filename.indexOf(']'));
}

static uint64_t game_start = 0;
static uint64_t pause_start;
static uint8_t menu_state = 0;
static bool use_diffname = false;
#define MENU_MAIN 0
#define MENU_LEVEL 1
#define MENU_DIFF 2
#define MENU_GAME 3
#define MENU_PAUSE 4
#define MENU_RESULTS 5
#define MENU_KEYBOARD 6
static std::vector<String> menu_options;
static unsigned menu_idx;
static String level_path;

Level lvl(""); // dummy level, we'll load the actual thing later on
lv_obj_t* score;
lv_obj_t* combo;
lv_obj_t* rating;
lv_obj_t* delta;
lv_obj_t* circles[2];
lv_obj_t* menu;
std::vector<lv_obj_t*> past;

static bool recalibrated = false;

static void menu_main() {
    menu_state = MENU_MAIN;
    menu_options.clear();
    menu_idx = 0;

    menu_options.push_back("play");
    menu_options.push_back("keyboard");
    // TODO
}
static void menu_level() {
    menu_state = MENU_LEVEL;
    menu_options.clear();
    menu_idx = 0;

    esd();
    maps_init();
    auto levels = maps_list();
    for(auto level : levels)
        menu_options.push_back(filename(level));
    maps_deinit();
    etft();
}
static void menu_diff() {
    menu_state = MENU_DIFF;
    menu_options.clear();
    menu_idx = 0;

    use_diffname = true;

    esd();
    maps_init();
    auto diffs = difficulty_list(level_path);
    for(auto diff : diffs)
        menu_options.push_back(filename(diff));
    maps_deinit();
    etft();
}
static void game_hide() {
    lv_obj_set_style_opa(score, LV_OPA_0, 0);
    lv_obj_set_style_opa(combo, LV_OPA_0, 0);
    lv_obj_set_style_opa(rating, LV_OPA_0, 0);
    lv_obj_set_style_opa(delta, LV_OPA_0, 0);
    lv_obj_set_style_opa(circles[0], LV_OPA_0, 0);
    lv_obj_set_style_opa(circles[1], LV_OPA_0, 0);
    lv_obj_set_style_opa(menu, LV_OPA_100, 0);
}
static void game_show() {
    lv_obj_set_style_opa(score, LV_OPA_100, 0);
    lv_obj_set_style_opa(combo, LV_OPA_100, 0);
    lv_obj_set_style_opa(rating, LV_OPA_100, 0);
    lv_obj_set_style_opa(delta, LV_OPA_100, 0);
    lv_obj_set_style_opa(circles[0], LV_OPA_100, 0);
    lv_obj_set_style_opa(circles[1], LV_OPA_100, 0);
    lv_obj_set_style_opa(menu, LV_OPA_0, 0);
}
static void menu_pause() {
    menu_state = MENU_PAUSE;
    menu_options.clear();
    menu_idx = 0;

    game_hide();
    menu_options.push_back("resume");
    menu_options.push_back("exit");

    for(auto x : past)
        lv_obj_del(x);
    past.clear();
}
static void menu_results() {
    menu_state = MENU_RESULTS;
    menu_options.clear();
    menu_idx = 0;

    game_hide();
    menu_options.push_back("GREAT: " + String(lvl.great));
    menu_options.push_back("OK: " + String(lvl.ok));
    menu_options.push_back("MISS: " + String(lvl.miss));
    menu_options.push_back("COMBO: " + String(lvl.combo));
    menu_options.push_back("MAX COMBO: " + String(lvl.maxcombo));
    menu_options.push_back("SCORE: " + String(lvl.score));
    menu_options.push_back("exit");

    for(auto x : past)
        lv_obj_del(x);
    past.clear();

    audio_stop();
}
static void menu_keyboard() {
    menu_state = MENU_KEYBOARD;
    menu_options.clear();
    menu_idx = 0;

    Keyboard.begin();
}
void setup() {
    // Serial.begin(115200); // no serial bc speaker

    audio_init();
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

    menu = lv_label_create(lv_scr_act());
    lv_obj_set_x(menu, 0);
    lv_obj_set_y(menu, 0);
    lv_obj_set_style_text_align(menu, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(menu, lv_color_white(), 0);
    lv_obj_set_width(menu, W);
    lv_obj_set_height(menu, H);
    lv_label_set_text(menu, "");

    int j = 0;
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
        circles[j++] = circle;
    }

    menu_main();
    game_hide();
}
uint8_t last_mask = 0;
static void render_menu(uint8_t pressed) {
    if(check_input(pressed, INPUT_KAT_LEFT)) {
        if(menu_idx == 0) menu_idx = menu_options.size() - 1;
        else menu_idx--;
    } else if(check_input(pressed, INPUT_KAT_RIGHT)) {
        if(menu_idx == menu_options.size() - 1) menu_idx = 0;
        else menu_idx++;
    }

    unsigned start = (menu_idx / 10) * 10;
    std::vector<String> sel_options;
    for(unsigned i = start; i < menu_options.size() && i < start + 10; i++)
        sel_options.push_back(menu_options[i]);
    String o = "";
    for(int i = 0; i < sel_options.size(); i++)
        o += (i == menu_idx - start ? "> " : "") + (use_diffname ? diffname(sel_options[i]) : sel_options[i]) + "\n";
    lv_label_set_text(menu, o.c_str());
}
static void loop_main(uint8_t pressed) {
    render_menu(pressed);

    if(check_input(pressed, INPUT_DON_RIGHT)) {
        String& sel = menu_options[menu_idx];
        if(sel == "play") menu_level();
        else if(sel == "keyboard") menu_keyboard();
    }
}
static void loop_level(uint8_t pressed) {
    render_menu(pressed);

    if(check_input(pressed, INPUT_DON_RIGHT)) {
        String& sel = menu_options[menu_idx];
        level_path = "/taiko/" + sel;
        menu_diff();
    } else if(check_input(pressed, INPUT_DON_LEFT))
        menu_main();
}
static void loop_diff(uint8_t pressed) {
    render_menu(pressed);

    if(check_input(pressed, INPUT_DON_RIGHT)) {
        use_diffname = false;
        String& sel = menu_options[menu_idx];
        esd();
        maps_init();
        lvl = load_level(level_path + "/" + sel);
        maps_deinit();
        etft();
        audio_play(level_path + "/");
        game_start = 0;
        menu_state = MENU_GAME;
        recalibrated = false;
        game_show();
    } else if(check_input(pressed, INPUT_DON_LEFT)) {
        use_diffname = false;
        menu_level();
    }
}
static void loop_game(uint8_t pressed) {
    if(!recalibrated && millis() - game_start >= 150) {
        recalibrated = true;
        recalibrate_input();
    }

    if(check_input(pressed, INPUT_BOOTSEL)) {
        pause_start = millis();
        menu_pause();

        return;
    }

    lvl.btn(millis() - game_start, pressed);

    for(auto x : past)
        lv_obj_del(x);
    past.clear();
    auto cur = lvl.render(millis() - game_start);
    if(cur.fin) {
        lvl.fin();
        menu_results();
        return;
    }
    for(auto x : cur.objs) {
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
    
    auto rat = lvl.get_rating(millis() - game_start);
    lv_label_set_text(rating, rat.txt.c_str());
    lv_label_set_text(delta, rat.delta.c_str());
    lv_obj_set_style_text_opa(rating, rat.opacity * 255, 0);
    lv_obj_set_style_text_opa(delta, rat.opacity * 255, 0);

    lv_label_set_text(score, String(lvl.score).c_str());
    lv_label_set_text(combo, String(lvl.combo).c_str());
}
static void loop_pause(uint8_t pressed) {
    render_menu(pressed);

    if(check_input(pressed, INPUT_DON_RIGHT)) {
        String& sel = menu_options[menu_idx];
        if(sel == "exit") {
            audio_stop();
            menu_main();
        } else if(sel == "resume") {
            game_show();
            game_start += millis() - pause_start;
            menu_state = MENU_GAME;
            menu_options.clear();
        }
    }
}
static void loop_results(uint8_t pressed) {
    render_menu(pressed);

    if(check_input(pressed, INPUT_DON_RIGHT)) {
        String& sel = menu_options[menu_idx];
        if(sel == "exit") {
            recalibrate_input();
            menu_main();
        }
    }
}
static void loop_keyboard(uint8_t held) {
    render_menu(0);

    if(check_input(held, INPUT_KAT_LEFT)) Keyboard.press('d');
    else Keyboard.release('d');
    if(check_input(held, INPUT_DON_LEFT)) Keyboard.press('f');
    else Keyboard.release('f');
    if(check_input(held, INPUT_DON_RIGHT)) Keyboard.press('j');
    else Keyboard.release('j');
    if(check_input(held, INPUT_KAT_RIGHT)) Keyboard.press('k');
    else Keyboard.release('k');
    if(check_input(held, INPUT_BOOTSEL)) {
        Keyboard.end();
        menu_main();
    }
}
uint64_t last = 0;
void loop() {
    if(menu_state == MENU_GAME) {
        audio_loop();
        if(game_start == 0) game_start = millis();
    }

    uint8_t mask = get_input();
    bool don = check_input(mask, INPUT_DON_LEFT) || check_input(mask, INPUT_DON_RIGHT);
    bool kat = check_input(mask, INPUT_KAT_LEFT) || check_input(mask, INPUT_KAT_RIGHT);
    // use 40 for dimmer led
    if(don && kat) p.neoPixelSetValue(0, 40, 40, 0, true);
    else if(kat) p.neoPixelSetValue(0, 0, 40, 40, true);
    else if(don) p.neoPixelSetValue(0, 40, 0, 0, true);
    else p.neoPixelSetValue(0, 0, 0, 0, true);

    uint8_t pressed = (mask ^ last_mask) & mask;
    // if(check_input(pressed, INPUT_DON_RIGHT)) Serial.println("DON_RIGHT");
    // if(check_input(pressed, INPUT_DON_LEFT)) Serial.println("DON_LEFT");
    // if(check_input(pressed, INPUT_KAT_RIGHT)) Serial.println("KAT_RIGHT");
    // if(check_input(pressed, INPUT_KAT_LEFT)) Serial.println("KAT_LEFT");
    // if(check_input(pressed, INPUT_BOOTSEL)) Serial.println("BOOTSEL");
    last_mask = mask;

    if(menu_state == MENU_MAIN)
        loop_main(pressed);
    else if(menu_state == MENU_LEVEL)
        loop_level(pressed);
    else if(menu_state == MENU_DIFF)
        loop_diff(pressed);
    else if(menu_state == MENU_GAME)
        loop_game(pressed);
    else if(menu_state == MENU_PAUSE)
        loop_pause(pressed);
    else if(menu_state == MENU_RESULTS)
        loop_results(pressed);
    else if(menu_state == MENU_KEYBOARD)
        loop_keyboard(mask);

    lv_refr_now(lv_disp_get_default());
    lv_timer_handler();
}