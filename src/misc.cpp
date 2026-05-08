#include "misc.h"

std::vector<String> split_string(String str, char sep, bool ignore_cr) {
    std::vector<String> out;
    String cur;
    for(char c : str) {
        if(c == sep) {
            out.push_back(cur);
            cur = "";
        } else if(!ignore_cr || c != '\r') cur += c;
    }
    out.push_back(cur);
    return out;
}