#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <SPI.h>
#include "maps.h"

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
            cfg.panel_width = 170;
            cfg.panel_height = 320;
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

Level lvl(""); // dummy level, we'll load the actual thing later on
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("init");

    etft();
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    esd();
    maps_init();

    etft();
    tft.setCursor(0, 0);
    esd();
    auto levels = maps_list();
    auto diffs = difficulty_list(levels[0]);
    lvl = load_level(diffs[0]);
    etft();
    tft.println(diffs.size());
    for(auto lv : levels)
        tft.println(lv);
    for(auto d : diffs)
        tft.println(d);

    delay(2000);
    tft.fillScreen(TFT_BLACK);
}
std::vector<LevelRenderObject> past;
void loop() {
    auto now = millis();
    auto cur = lvl.render(millis());
    for(auto x : past) {
        int r = x.big ? 35 : 20;
        tft.fillCircle(x.x, 170 / 2, r, TFT_BLACK);
    }
    for(auto x : past = cur) {
        int r = x.big ? 35 : 20;
        int c = x.kat ? TFT_CYAN : TFT_RED;
        tft.fillCircle(x.x, 170 / 2, r, c);
    }
    tft.startWrite();
    tft.endWrite();
    tft.flush();

    delay(33 - millis() + now);
}