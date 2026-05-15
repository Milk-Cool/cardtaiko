#include "audio.h"
#include "en.h"
#include <SDFS.h>
#include <PWMAudio.h>
#include <AudioGeneratorMP3.h>
#include <AudioFileSourceSD.h>
#include <AudioOutputPWM.h>
#include <stdlib.h>

static String path;
static AudioFileSourceSD* src;
static AudioGeneratorMP3* mp3;
static AudioOutputPWM* pwm;
static uint8_t filebuff[512];
void audio_init() {
    src = nullptr;
    pwm = new AudioOutputPWM(44100, 0);
    mp3 = nullptr;
    // mp3.setGain(0.2);
}
void audio_play(String diff_path) {
    path = diff_path.substring(0, diff_path.lastIndexOf('/')) + "/audio.mp3";
    esd();
    if(src != nullptr) {
        delete src;
    }
    src = new AudioFileSourceSD(path.c_str());
    if(mp3 != nullptr) {
        mp3->stop();
        delete mp3;
    }
    mp3 = new AudioGeneratorMP3();
    mp3->begin(src, pwm);
    etft();
}
void audio_loop() {
    if(!mp3->isRunning()) return;
    esd();
    if(!mp3->loop()) mp3->stop();
    etft();
}