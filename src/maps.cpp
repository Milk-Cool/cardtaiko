#include "maps.h"
#include <SDFS.h>

bool maps_init() {
    SDFS.setConfig(SDFSConfig(5, SPI_QUARTER_SPEED, SPI1));
    bool ret = SDFS.begin();
    if(ret) {
        if(!SDFS.exists("/taiko")) SDFS.mkdir("/taiko");
    }
    return ret;
}

std::vector<String> maps_list() {
    File maps = SDFS.open("/taiko", "r");
    std::vector<String> out;
    while(true) {
        File map = maps.openNextFile();
        if(!map) break;
        if(map.isDirectory())
            out.push_back(map.fullName());
        map.close();
    }
    return out;
}
std::vector<String> difficulty_list(String path) {
    File maps = SDFS.open(path, "r");
    std::vector<String> out;
    while(true) {
        File map = maps.openNextFile();
        if(!map) break;
        if(String(map.name()).endsWith(".osu"))
            out.push_back(map.fullName());
        map.close();
    }
    return out;
}
Level load_level(String path) {
    File f = SDFS.open(path, "r");
    String contents = f.readString();
    f.close();
    Level lv(contents);
    return lv;
}