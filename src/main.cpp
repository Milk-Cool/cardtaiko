#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include "maps.h"

Adafruit_ST7789 tft = Adafruit_ST7789(&SPI1, 13, 27, 29);

Level lvl(""); // dummy level, we'll load the actual thing later on
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("init");
    tft.init(170, 320);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_BLACK);
    maps_init();

    auto levels = maps_list();
    for(auto lv : levels)
        tft.println(lv);
    auto diffs = difficulty_list(levels[0]);
    for(auto d : diffs)
        tft.println(d);
    lvl = load_level(diffs[0]);

    delay(5000);
    tft.fillScreen(ST77XX_BLACK);
}
void loop() {
    tft.fillRect(0, 170 / 2 - 60, 320, 120, ST77XX_BLACK);
    for(auto x : lvl.render(millis())) {
        int r = x.big ? 50 : 30;
        int c = x.kat ? ST77XX_CYAN : ST77XX_RED;
        tft.fillCircle(x.x, 170 / 2, r, c);
    }
    delay(20);
}