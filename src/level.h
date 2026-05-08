#pragma once
#include <Arduino.h>
#include <vector>

typedef struct {
    double time;
    uint8_t type;
    uint8_t sound;
} LevelHitObject;
typedef struct {
    int x;
    uint8_t type;
    bool kat;
    bool big;
} LevelRenderObject;
class Level {
    public:
        Level(String txt);
        std::vector<LevelRenderObject> render(double t);
    private:
        std::vector<LevelHitObject> hit_objects;
};