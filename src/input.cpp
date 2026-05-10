#include "input.h"
#include <Arduino.h>

void init_input() {
    pinMode(1, INPUT_PULLUP);
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP);
}
uint8_t get_input() {
    uint8_t mask = 0;
    if(!digitalRead(1)) mask |= 1 << INPUT_KAT_RIGHT;
    if(!digitalRead(2)) mask |= 1 << INPUT_DON_RIGHT;
    if(!digitalRead(3)) mask |= 1 << INPUT_DON_LEFT;
    if(!digitalRead(4)) mask |= 1 << INPUT_KAT_LEFT;
    if(BOOTSEL) mask |= 1 << INPUT_BOOTSEL;
    return mask;
}
bool check_input(uint8_t mask, uint8_t bit) {
    return !!(mask & (1 << bit));
}