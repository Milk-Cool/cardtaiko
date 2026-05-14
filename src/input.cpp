#include "input.h"
#include <Arduino.h>
#include <TouchyTouch.h>

TouchyTouch touch[4];
void init_input() {
    for(int i = 0; i < 4; i++) {
        // pins 1-4
        touch[i].begin(i + 1);
        // touch[i].threshold += 100; // from example code
    }
}
uint8_t get_input() {
    for(int i = 0; i < 4; i++) touch[i].update();

    uint8_t mask = 0;
    if(touch[0].touched()) mask |= 1 << INPUT_KAT_RIGHT;
    if(touch[1].touched()) mask |= 1 << INPUT_DON_RIGHT;
    if(touch[2].touched()) mask |= 1 << INPUT_DON_LEFT;
    if(touch[3].touched()) mask |= 1 << INPUT_KAT_LEFT;
    if(BOOTSEL) mask |= 1 << INPUT_BOOTSEL;
    return mask;
}
bool check_input(uint8_t mask, uint8_t bit) {
    return !!(mask & (1 << bit));
}