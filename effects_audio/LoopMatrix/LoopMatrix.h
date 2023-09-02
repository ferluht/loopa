//
// Created by ferluht on 03/12/2022.
//

#ifndef RPIDAW_LOOPMATRIX_H
#define RPIDAW_LOOPMATRIX_H

#include <Effect.h>
#include <Tape/Tape.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <Rack.h>
#include <AudioEffects.h>

class LoopMatrix : public AudioEffect {

    static const int m = 2;
    static const int n = 2;

    Tape * tapes[m*n];
    bool saved[m*n];
    int saveiterations = 0;
    std::string savedir;
    int focus_tape;
    Tape * copy_from;
    Sync * sync;
    int draw_counter = 0;
    int current_screen = 0;
    int scene_idx = 0;

    Rack * master_effects;
    Parameter sends[m*n][2];
    bool scene_copying_status[m*n];

public:

    LoopMatrix();
    void process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, double streamTime) override;
    void draw(GFXcanvas1 * screen) override;
    void draw_main_screen(GFXcanvas1 * screen);
    void draw_fx_screen(GFXcanvas1 * screen);
    inline void select_screen(int idx) { current_screen = idx; }
    bool save();
};


#endif //RPIDAW_LOOPMATRIX_H
