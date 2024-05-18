//
// Created by ferluht on 28/07/2022.
//

#pragma once

#include "AMG.h"
#include <MidiEffects.h>
#include <iostream>

class Rack : public AMG {

public:

    enum RACKTYPE {
        PARALLEL,
        SEQUENTIAL,
        SELECTIVE,
    };

    Rack(RACKTYPE racktype_) : AMG("Rack") {
        racktype = racktype_;
        focus_item = items.begin();
        if (racktype == PARALLEL) emptybuffer = new float[AUDIO_BUF_SIZE * 2];
        else emptybuffer = nullptr;
        outbuffer = nullptr;

        addDrawHandler({SCREENS::LOOP_VIEW}, [this](GFXcanvas1 * screen) -> void {
            if (focus_item != items.end())
                (*focus_item)->draw(screen);
        });
    }

    void midiIn(MData& cmd, Sync & sync) override;
    void midiOut(std::deque<MData> &q, Sync & sync) override;

    void process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, Sync & sync) override;

    void add(AMG * item);

    Rack * dive_prev();
    Rack * dive_next();
    void set_focus_by_index(int i);
    AMG * get_focus();
    AMG * get_back();
    int get_focus_index();
    inline AMG * get_item(int i) { return items[i % items.size()]; }
    inline int get_size() { return items.size(); }

private:

    RACKTYPE racktype;
    std::vector<AMG*> items;
    std::vector<AMG*>::iterator focus_item;

    float * outbuffer;
    float * emptybuffer;
};
