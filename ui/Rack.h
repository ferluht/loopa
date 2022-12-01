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

    Rack(const char * name, RACKTYPE racktype_) : AMG(name) {
        racktype = racktype_;
        focus_item = items.begin();
        if (racktype == PARALLEL) emptybuffer = new float[BUF_SIZE * 2];
        else emptybuffer = nullptr;
        outbuffer = nullptr;
        parent = nullptr;
        rackMidiStatus = MIDISTATUS::DONE;
    }

    MIDISTATUS midiIn(MData& cmd) override;

    void draw(GFXcanvas1 * screen) override;

    void process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, double streamTime) override;

    void add(AMG * item);

    void attach(Rack * parent_);

    Rack * dive_in();
    Rack * dive_out();
    Rack * dive_prev();
    Rack * dive_next();
    void set_focus_by_index(int i);
    AMG * get_focus();
    int get_focus_index();

private:

    RACKTYPE racktype;
    std::vector<std::pair<AMG*, MIDISTATUS>> items;
    std::vector<std::pair<AMG*, MIDISTATUS>>::iterator focus_item;
    MIDISTATUS rackMidiStatus;

    Rack * parent;

    float * outbuffer;
    float * emptybuffer;
};
