//
// Created by ferluht on 28/07/2022.
//

#pragma once

#include <AMG.h>

class Rack : public AMG {

public:

    enum RACKTYPE {
        PARALLEL,
        SEQUENTIAL,
        SELECTIVE,
    };

    Rack(RACKTYPE racktype_) {
        racktype = racktype_;
        focus_item = items.begin();
        if (racktype == PARALLEL) emptybuffer = new float[BUF_SIZE * 2];
        else emptybuffer = nullptr;
        outbuffer = nullptr;
        parent = nullptr;
    }

    void midiIn(MData& cmd) override;

    void draw(NVGcontext * vg) override;

    void process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, double streamTime) override;

    void add(AMG * item);

    void attach(Rack * parent_);

    Rack * dive_in();
    Rack * dive_out();
    void dive_next();
    void set_focus_by_index(int i);
    AMG * get_focus();

private:

    RACKTYPE racktype;
    std::vector<AMG*> items;
    std::vector<AMG*>::iterator focus_item;

    Rack * parent;

    float * outbuffer;
    float * emptybuffer;
};
