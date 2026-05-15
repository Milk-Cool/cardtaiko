#include "en.h"
#include <Arduino.h>
#include <SPI.h>

void etft() {
    SPI1.endTransaction();
    SPI1.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));
    digitalWrite(13, LOW);
    digitalWrite(5, HIGH);
}
void esd() {
    SPI1.endTransaction();
    SPI1.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(13, HIGH);
    digitalWrite(5, LOW);
}