#include "level.h"
#include "misc.h"
#include <algorithm>

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
                .sound = split[4].toInt()
            });
        } else if(category == "[Difficulty]") {
            auto split = split_string(str, ':', false);
            if(split[0] == "OverallDifficulty") overall_difficulty = split[1].toInt();
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
            if(i - 1== x) {
                flag = true;
                break;
            }
        if(flag) continue;
        
        int x = (obj.time - t) / 3 + 40;
        if(x > -40 && x < 360) ret.push_back((LevelRenderObject) {
            .x = x,
            .type = obj.type,
            .kat = (obj.sound & 2) || (obj.sound & 8),
            .big = obj.sound & 4
        });
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