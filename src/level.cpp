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

    int sum = 0, i = 0;
    for(auto obj& : hit_objects) {
        if(hit_objects[i].type & 2) continue;
        i++;
        sum += min(i / 10, 10);
    }
    combo_bonus = ceil(100000.0 / sum) * 10;
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
        for(auto x : miss_idx)
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
            res.len = (double)obj.len / 2 * lendiv; // 640 = 2 * 320, DisplayWidth = 320
        }
        ret.push_back(res);
    }
    std::reverse(ret.begin(), ret.end());
    return ret;
}
Rating Level::get_rating(double t) {
    if(t - rating_time > 400) return (Rating) { .txt = "", .opacity = 0 };
    return (Rating) { .txt = rating_txt, .opacity = t - rating_time <= 200 ? 1 : 1 - (t - rating_time - 200) / 200 };
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
unsigned Level::calc_score(unsigned base, bool big) {
    return (base + combo_bonus * min(10, combo / 10)) * (big ? 2 : 1);
}
#define TXT_GREAT { rating_txt = "GREAT"; rating_time = t; }
#define TXT_OK { rating_txt = "OK"; rating_time = t; }
#define TXT_MISS { rating_txt = "MISS"; rating_time = t; }
void Level::btn(double t, uint8_t button) {
    // if(!button) return;

    for(unsigned i = 0; i < cached.size(); i++) {
        LevelHitObject& obj = hit_objects[cached[i].i];
        if(t - cached[i].t > 60) {
            if(obj.type & 2) {
                score += 300;
            } else {
                if(is_great(abs(cached[i].t - obj.time))) { great++; score += calc_score(300, false); combo++; TXT_GREAT }
                else if(is_ok(abs(cached[i].t - obj.time))) { ok++; score += calc_score(100, false); combo++; TXT_OK }
                else { miss++; combo = 0; TXT_MISS }
                hit_idx.push_back(cached[i].i);
            }
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
            if(obj.type & 2) {
                // drumroll
                score += (obj.sound & 4) ? 600 : 300;
            } else {
                if(is_great(abs(cached[i].t - obj.time))) { great++; score += calc_score(300, true); combo++; TXT_GREAT }
                else if(is_ok(abs(cached[i].t - obj.time))) { ok++; score += calc_score(100, true); combo++; TXT_OK }
                else { miss++; combo = 0; TXT_MISS }
                hit_idx.push_back(cached[i].i);
            }
            cached.erase(cached.begin() + i);
            i--;
        }
    }

    unsigned i = 0;
    unsigned min_idx = 0xffffffff, min_val = 501;
    unsigned max_idx = 0xffffffff, max_val = 0;
    bool min_is_big = false, min_is_kat = false;
    bool max_is_big = false, max_is_kat = false;
    for(auto obj : hit_objects) {
        i++;
        double diff = abs(obj.time - t);
        bool kat = (obj.sound & 2) || (obj.sound & 8);
        bool big = obj.sound & 4;
        if(!is_miss(diff) && !(obj.type & 2)) {
            if(obj.time < t) {
                bool flag = false;
                for(auto x : miss_idx)
                    if(i - 1 == x) {
                        flag = true;
                        break;
                    }
                for(auto x : hit_idx)
                    if(i - 1 == x) {
                        flag = true;
                        break;
                    }
                if(!flag) {
                    miss_idx.push_back(i - 1);
                    combo = 0;
                }
            }
            continue;
        }
        if(!(obj.type & 2) && kat && !check_input(button, INPUT_KAT_LEFT) && !check_input(button, INPUT_KAT_RIGHT)) continue;
        if(!(obj.type & 2) && !kat && !check_input(button, INPUT_DON_LEFT) && !check_input(button, INPUT_DON_RIGHT)) continue;
        if((obj.type & 2) && !check_input(button, INPUT_KAT_LEFT) && !check_input(button, INPUT_KAT_RIGHT) && !check_input(button, INPUT_DON_LEFT) && !check_input(button, INPUT_DON_RIGHT)) continue;
        if(obj.time < t && max_val < obj.time && (obj.type & 2)) {
            max_idx = i - 1;
            max_val = obj.time;
            max_is_big = big;
            max_is_kat = kat;
        }

        bool flag = false;
        for(auto x : hit_idx)
            if(i - 1 == x) {
                flag = true;
                break;
            }
        for(auto x : miss_idx)
            if(i - 1 == x) {
                flag = true;
                break;
            }
        if(flag) continue;

        if(diff < min_val && !(obj.type & 2)) { // delibirately ignore sliders
            min_idx = i - 1;
            min_val = diff;
            min_is_big = big;
            min_is_kat = kat;
        }
    }
    if(min_idx == 0xffffffff) {
        if(max_idx == 0xffffffff) return;
        LevelHitObject& obj = hit_objects[max_idx];
        int x = (obj.time - t) / lendiv + obj.len / 2 * lendiv;
        if(x >= 0) {
            if(max_is_big)
                cached.push_back((CachedDoubleHit) {
                    .t = t,
                    .i = max_idx,
                    .m = button,
                    .kat = max_is_kat
                });
            else {
                score += 300;
            }
        }
        return;
    }

    if(min_is_big && !((button & INPUT_DON_LEFT) && (button & INPUT_DON_RIGHT)) && !((button & INPUT_KAT_LEFT) && (button & INPUT_KAT_RIGHT))) {
        cached.push_back((CachedDoubleHit) {
            .t = t,
            .i = min_idx,
            .m = button,
            .kat = min_is_kat
        });
        return;
    }

    // OD & timing windows
    // https://osu.ppy.sh/wiki/en/Beatmap/Overall_difficulty
    if(is_great(min_val)) { great++; score += calc_score(300, min_is_big); combo++; TXT_GREAT }
    else if(is_ok(min_val)) { ok++; score += calc_score(100, min_is_big); combo++; TXT_OK }
    else { miss++; combo = 0; TXT_MISS }
    hit_idx.push_back(min_idx);
}