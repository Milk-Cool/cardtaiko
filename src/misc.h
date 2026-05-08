#pragma once
#include <Arduino.h>
#include <vector>

std::vector<String> split_string(String str, char sep = ',', bool ignore_cr = false);