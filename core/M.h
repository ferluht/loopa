//
// Created by ferluht on 08/07/2022.
//

#pragma once

#include <stdlib.h>
#include <vector>
#include <queue>
#include <cmath>
#include <list>
#include <string>

#define NOTEOFF_HEADER 128
#define NOTEON_HEADER 144
#define POLYPRESSURE_HEADER 160
#define CC_HEADER 176
#define PITCHWHEEL_HEADER 224

#define CC_MODWHEEL 1
#define CC_BREATH_CONTROLLER 2
#define CC_AFTERTOUCH 3
#define CC_FOOT_CONTROLLER 4
#define CC_PORTAMENTO_TIME 5
#define CC_DATA_ENTRY 6
#define CC_MAIN_VOLUME 7
#define CC_DAMPER_PEDAL 64
#define CC_PORTAMENTO 65
#define CC_SOSTENUTO 66
#define CC_SOFT_PEDAL 67
#define CC_CHORUS 93

#define CC_E1 105
#define CC_E2 106

#define S1 100
#define S2 101

#define K1 60
#define K2 61
#define K3 62
#define K4 63
#define K5 64
#define K6 65
#define K7 66

#define P1 110
#define P2 111
#define P3 112
#define P4 113

struct MData {
    double beat;
    unsigned char status;
    unsigned char data1;
    unsigned char data2;
};

class M {
public:

    M() {

    }

    virtual void midiIn(MData& cmd) {}
};
