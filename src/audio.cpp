#include "audio.h"
#include "en.h"
#include <SDFS.h>
#include <PWMAudio.h>
#include <AudioGeneratorMP3.h>
#include <AudioFileSourceSD.h>
#include <AudioFileSourceBuffer.h>
#include <AudioOutputPWM.h>
#include <stdlib.h>

#define BSIZE 65536
static String path;
static AudioFileSourceSD* src;
static AudioFileSourceBuffer* buf;
static AudioGeneratorMP3* mp3;
static AudioOutputPWM* pwm;
void audio_init() {
    src = nullptr;
    pwm = new AudioOutputPWM(44100, 0);
    mp3 = nullptr;
    buf = nullptr;
    // mp3.setGain(0.2);
}
void audio_play(String diff_path) {
    path = diff_path.substring(0, diff_path.lastIndexOf('/')) + "/audio.mp3";
    esd();
    SD.begin(5, 10000000, SPI1);
    if(src != nullptr) {
        delete src;
    }
    if(buf != nullptr) {
        // buf->close();
        delete buf;
    }
    src = new AudioFileSourceSD(path.c_str());
    buf = new AudioFileSourceBuffer(src, BSIZE);
    if(mp3 != nullptr) {
        // mp3->stop();
        delete mp3;
    }
    mp3 = new AudioGeneratorMP3();
    mp3->begin(buf, pwm);
    pwm->SetGain(0.5);
    etft();
}
int audio_loop() {
    if(!mp3) return -1;
    if(!mp3->isRunning()) return -2;
    esd();
    if(!mp3->loop()) mp3->stop();
    etft();
    return 0;
}
void audio_stop() {
    esd();
    mp3->stop();
    SD.end();
    etft();
}