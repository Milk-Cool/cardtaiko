#pragma once
#include <Arduino.h>
#include <vector>
#include "input.h"

typedef struct {
    double time;
    uint8_t type;
    uint8_t sound;
    uint16_t len;
    uint8_t denden;
    uint16_t denden_cnt;
} LevelHitObject;
typedef struct {
    int x;
    uint8_t type;
    bool kat;
    bool big;
    uint16_t len;
} LevelRenderObject;
typedef struct {
    double t;
    unsigned i;
    uint8_t m;
    bool kat;
} CachedDoubleHit;
typedef struct {
    double t;
    double l;
    bool uninherited;
} TimingPoint;
typedef struct {
    String txt;
    String delta;
    float opacity;
} Rating;
class Level {
    public:
        Level(String txt);
        std::vector<LevelRenderObject> render(double t);
        Rating get_rating(double t);
        int8_t overall_difficulty = 0;
        unsigned combo = 0;
        unsigned great = 0;
        unsigned ok = 0;
        unsigned miss = 0;
        unsigned score = 0;
        void btn(double t, uint8_t button);
    private:
        std::vector<LevelHitObject> hit_objects;
        std::vector<uint16_t> hit_idx;
        std::vector<uint16_t> miss_idx;
        std::vector<CachedDoubleHit> cached;
        bool is_great(double t);
        bool is_ok(double t);
        bool is_miss(double t);
        double slider_mult;
        unsigned calc_score(unsigned base, bool big);
        std::vector<TimingPoint> timings;
        String rating_txt = "";
        String delta_txt = "";
        double rating_time = 0;
        int combo_bonus = 0;
};