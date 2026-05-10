#pragma once
#include <stdint.h>

#define INPUT_KAT_RIGHT 0
#define INPUT_DON_RIGHT 1
#define INPUT_DON_LEFT 2
#define INPUT_KAT_LEFT 3
#define INPUT_BOOTSEL 7

void init_input();
uint8_t get_input();
bool check_input(uint8_t mask, uint8_t bit);