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
void Level::hit(double t, uint8_t button) {
    if(button & ~INPUT_BOOTSEL == 0) return;

    unsigned i = 0;
    unsigned min_idx = 0xffffffff, min_val = 501;
    for(auto obj : hit_objects) {
        i++;
        double diff = abs(obj.time - t);
        if(diff > (overall_difficulty <= 5 ? 135 - 8 * overall_difficulty : 120 - 5 * overall_difficulty)) continue;
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
        }
    }
    if(min_idx == 0xffffffff) return;

    // OD & timing windows
    // https://osu.ppy.sh/wiki/en/Beatmap/Overall_difficulty
    if(min_val <= 50 - 3 * overall_difficulty)
        great++;
    else if(min_val <= (overall_difficulty <= 5 ? 120 - 8 * overall_difficulty : 110 - 6 * overall_difficulty))
        ok++;
    else miss++;
    hit_idx.push_back(min_idx);
}