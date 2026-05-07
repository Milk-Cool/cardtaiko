#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

Adafruit_ST7789 tft = Adafruit_ST7789(&SPI1, 13, 27, 29);

void setup() {
    Serial.begin(115200);
    Serial.println("init");
    tft.init(170, 320);
    tft.setRotation(1);
}
void loop() {
    tft.fillScreen(ST77XX_WHITE);
}