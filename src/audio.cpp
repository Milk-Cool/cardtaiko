#include "audio.h"
#include "en.h"
#include <SDFS.h>
#include <PWMAudio.h>
#include <BackgroundAudio.h>

static String path;
static size_t idx;
static PWMAudio pwm(0);
static BackgroundAudioMP3 mp3(pwm);
static uint8_t filebuff[512];
void audio_init() {
    mp3.begin();
}
void audio_play(String diff_path) {
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
}
void audio_loop() {
    if(path == "") return;
    esd();
    File f = SDFS.open(path, "r");
    f.seek(idx);
    while(f && mp3.availableForWrite() > 512) {
        int len = f.read(filebuff, 512);
        mp3.write(filebuff, len);
        idx += len;
        if(len != 512) f.close();
    }
    etft();
}