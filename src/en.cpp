#include "en.h"
#include <Arduino.h>

void etft() {
    digitalWrite(13, LOW);
    digitalWrite(5, HIGH);
}
void esd() {
    digitalWrite(13, HIGH);
    digitalWrite(5, LOW);
}