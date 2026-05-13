#include "audio.h"
#include "en.h"
#include <SDFS.h>

static String path;
static size_t idx;
static BackgroundAudioMP3* g_mp3;
static uint8_t filebuff[512];
void audio_play(String diff_path, BackgroundAudioMP3& mp3) {
    path = diff_path.substring(0, diff_path.lastIndexOf('/')) + "/audio.mp3";
    idx = 0;
    esd();
    File f = SDFS.open(path, "r");
    if(!f) {
        etft();
        path = "";
        return;
    }
    f.close();
    etft();
    g_mp3 = &mp3;
}
void audio_loop() {
    if(path == "") return;
    esd();
    File f = SDFS.open(path, "r");
    f.seek(idx);
    while(f && g_mp3->availableForWrite() > 512) {
        int len = f.read(filebuff, 512);
        g_mp3->write(filebuff, len);
        idx += len;
        if(len != 512) f.close();
    }
    etft();
}