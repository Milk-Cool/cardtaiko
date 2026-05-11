#pragma once
#include <Arduino.h>
#include <vector>
#include "level.h"

bool maps_init();
void maps_deinit();
std::vector<String> maps_list();
std::vector<String> difficulty_list(String path);
Level load_level(String path);