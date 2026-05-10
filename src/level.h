#pragma once
#include <Arduino.h>
#include <vector>
#include "input.h"

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
typedef struct {
    double t;
    unsigned i;
    uint8_t m;
    bool kat;
} CachedDoubleHit;
class Level {
    public:
        Level(String txt);
        std::vector<LevelRenderObject> render(double t);
        int8_t overall_difficulty = 0;
        unsigned combo = 0;
        unsigned great = 0;
        unsigned ok = 0;
        unsigned miss = 0;
        void btn(double t, uint8_t button);
    private:
        std::vector<LevelHitObject> hit_objects;
        std::vector<uint16_t> hit_idx;
        std::vector<CachedDoubleHit> cached;
        bool is_great(double t);
        bool is_ok(double t);
        bool is_miss(double t);
};