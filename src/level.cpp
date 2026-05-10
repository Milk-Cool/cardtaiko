// https://osu.ppy.sh/wiki/en/Client/File_formats/osu_%28file_format%29

#include "level.h"
#include "misc.h"
#include <algorithm>

static int lendiv = 3;

Level::Level(String txt) {
    auto split = split_string(txt, '\n', true);
    String category;
    for(String str : split) {
        if(str == "") continue;
        if(str[0] == '[') {
            category = str;
            continue;
        }
        if(category == "[HitObjects]") {
            auto split = split_string(str, ',', false);
            hit_objects.push_back((LevelHitObject) {
                .time = split[2].toDouble(),
                .type = split[3].toInt(),
                .sound = split[4].toInt(),
                .len = split[3].toInt() & 2 ? split[7].toFloat() : 0
            });
        } else if(category == "[Difficulty]") {
            auto split = split_string(str, ':', false);
            if(split[0] == "OverallDifficulty") overall_difficulty = split[1].toInt();
        } else if(category == "[TimingPoints]") {
            auto split = split_string(str, ',', false);
            timings.push_back((TimingPoint) {
                .t = split[0].toDouble(),
                .l = split[1].toDouble(),
                .uninherited = split[6] == "1"
            });
        }
    }
}
std::vector<LevelRenderObject> Level::render(double t) {
    std::vector<LevelRenderObject> ret;
    unsigned i = 0;
    for(auto obj : hit_objects) {
        i++;
        if(abs(obj.time - t) >= 2000) continue;
        bool flag = false;
        for(auto x : hit_idx)
            if(i - 1 == x) {
                flag = true;
                break;
            }
        if(flag) continue;
        
        int minx = (obj.time - t) / lendiv + 40;
        int maxx = (obj.time - t) / lendiv + 40 + obj.len / 2 * lendiv;
        if(maxx < -40 || minx > 360) continue;
        LevelRenderObject res = {
            .x = minx,
            .type = obj.type,
            .kat = (obj.sound & 2) || (obj.sound & 8),
            .big = obj.sound & 4
        };
        if(obj.type & 2) {
            // int16_t beat_dur = 0;
            // for(auto timing : timings) {
            //     if(timing.t > obj.time + 1) break;
            //     if(timing.uninherited) beat_dur = timing.l;
            //     else beat_dur *= -timing.l / 100;
            // }
            res.len = (double)obj.len / 2 * lendiv; // 640 = 2 * 320, DisplayWidth = 320
        }
        ret.push_back(res);
    }
    std::reverse(ret.begin(), ret.end());
    return ret;
}
bool Level::is_great(double t) {
    return t <= 50 - 3 * overall_difficulty;
}
bool Level::is_ok(double t) {
    return t <= (overall_difficulty <= 5 ? 120 - 8 * overall_difficulty : 110 - 6 * overall_difficulty);
}
bool Level::is_miss(double t) {
    return t <= (overall_difficulty <= 5 ? 135 - 8 * overall_difficulty : 120 - 5 * overall_difficulty);
}
void Level::btn(double t, uint8_t button) {
    if(button & ~INPUT_BOOTSEL == 0) return;

    for(unsigned i = 0; i < cached.size(); i++) {
        if(t - cached[i].t > 60) {
            if(is_great(abs(cached[i].t - hit_objects[cached[i].i].time))) great++;
            else if(is_ok(abs(cached[i].t - hit_objects[cached[i].i].time))) ok++;
            hit_idx.push_back(cached[i].i);
            cached.erase(cached.begin() + i);
            i--;
            continue;
        }
        bool hit = false;
        if(cached[i].kat && check_input(cached[i].m, INPUT_KAT_LEFT) && check_input(button, INPUT_KAT_RIGHT)) hit = true;
        if(cached[i].kat && check_input(cached[i].m, INPUT_KAT_RIGHT) && check_input(button, INPUT_KAT_LEFT)) hit = true;
        if(!cached[i].kat && check_input(cached[i].m, INPUT_DON_LEFT) && check_input(button, INPUT_DON_RIGHT)) hit = true;
        if(!cached[i].kat && check_input(cached[i].m, INPUT_DON_RIGHT) && check_input(button, INPUT_DON_LEFT)) hit = true;
        if(hit) {
            if(is_great(abs(cached[i].t - hit_objects[cached[i].i].time))) great++;
            else if(is_ok(abs(cached[i].t - hit_objects[cached[i].i].time))) ok++;
            hit_idx.push_back(cached[i].i);
            cached.erase(cached.begin() + i);
            i--;
        }
    }

    unsigned i = 0;
    unsigned min_idx = 0xffffffff, min_val = 501;
    bool is_big = false, is_kat = false;
    for(auto obj : hit_objects) {
        i++;
        double diff = abs(obj.time - t);
        if(!is_miss(diff)) continue;
        bool kat = (obj.sound & 2) || (obj.sound & 8);
        bool big = obj.sound & 4;
        if(kat && !check_input(button, INPUT_KAT_LEFT) && !check_input(button, INPUT_KAT_RIGHT)) continue;
        if(!kat && !check_input(button, INPUT_DON_LEFT) && !check_input(button, INPUT_DON_RIGHT)) continue;

        bool flag = false;
        for(auto x : hit_idx)
            if(i - 1 == x) {
                flag = true;
                break;
            }
        if(flag) continue;

        if(diff < min_val) {
            min_idx = i - 1;
            min_val = diff;
            is_big = big;
            is_kat = kat;
        }
    }
    if(min_idx == 0xffffffff) return;

    if(is_big) {
        cached.push_back((CachedDoubleHit) {
            .t = t,
            .i = min_idx,
            .m = button,
            .kat = is_kat
        });
        return;
    }

    // OD & timing windows
    // https://osu.ppy.sh/wiki/en/Beatmap/Overall_difficulty
    if(is_great(min_val))
        great++;
    else if(is_ok(min_val))
        ok++;
    else miss++;
    hit_idx.push_back(min_idx);
}