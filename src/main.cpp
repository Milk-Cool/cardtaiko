#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <SDFS.h>

Adafruit_ST7789 tft = Adafruit_ST7789(&SPI1, 13, 27, 29);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("init");
    tft.init(170, 320);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_WHITE);

    SDFS.setConfig(SDFSConfig(5, SPI_QUARTER_SPEED, SPI1));
    SDFS.begin();
    File root = SDFS.open("/", "r");
    tft.setTextColor(ST77XX_BLACK);
    tft.setCursor(30, 0);
    Serial.println(String(!!root));
    tft.print(String(!!root));
    int i = 1;
    while(true) {
        File file = root.openNextFile();
        if(!file) break;
        tft.setCursor(0, i++ * 30);
        tft.print(file.name());
        file.close();
    }
}
void loop() {
}