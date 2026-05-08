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
        }
    }
}
std::vector<LevelRenderObject> Level::render(double t) {
    std::vector<LevelRenderObject> ret;
    t /= 2; // px to ms ratio
    for(auto obj : hit_objects) {
        if(abs(obj.time - t) < 1000) ret.push_back((LevelRenderObject) {
            .x = obj.time - t + 50,
            .type = obj.type,
            .kat = (obj.sound & 2) || (obj.sound & 8),
            .big = obj.sound & 4
        });
    }
    std::reverse(ret.begin(), ret.end());
    return ret;
}