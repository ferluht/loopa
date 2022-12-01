//
// Created by ferluht on 28/07/2022.
//

#include "Rack.h"

MIDISTATUS Rack::midiIn(MData &cmd) {
    switch (racktype) {
        case PARALLEL:
            for (auto it = items.begin(); it < items.end(); it++) {
                MData ccmd = cmd;
                if (rackMidiStatus != MIDISTATUS::WAITING || it->second != MIDISTATUS::DONE)
                    it->second = it->first->midiIn(ccmd);
            }
            break;
        case SEQUENTIAL:
            for (auto it = items.begin(); it < items.end(); it++) {
                if (rackMidiStatus != MIDISTATUS::WAITING || it->second != MIDISTATUS::DONE)
                    it->second = it->first->midiIn(cmd);
            }
            break;
        case SELECTIVE:
            if (focus_item != items.end())
                focus_item->second = focus_item->first->midiIn(cmd);
            break;
        default:
            break;
    }
    for (auto it = items.begin(); it < items.end(); it++) {
        if (it->second != MIDISTATUS::DONE) {
            rackMidiStatus = MIDISTATUS::WAITING;
            return MIDISTATUS::WAITING;
        }
    }
    return MIDISTATUS::DONE;
}

void Rack::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, double streamTime) {
    switch (racktype) {
        case PARALLEL:
            for (unsigned int i = 0; i < 2 * nBufferFrames; i++) emptybuffer[i] = 0;

            for (auto it = items.begin(); it < items.end(); it++) {
                if (it == focus_item)
                    it->first->process(&outbuffer[(it - items.begin()) * BUF_SIZE * 2], inputBuffer, nBufferFrames, 0);
                else
                    it->first->process(&outbuffer[(it - items.begin()) * BUF_SIZE * 2], emptybuffer, nBufferFrames, 0);
            }

            for (unsigned int k = 0; k < items.size(); k ++)
                for (unsigned int i = 0; i < 2 * nBufferFrames; i ++)
                    outputBuffer[i] += outbuffer[k * BUF_SIZE * 2 + i];
            break;
        case SEQUENTIAL:
            for (auto it = items.begin(); it < items.end(); it++) {
                it->first->process(outputBuffer, inputBuffer, nBufferFrames, streamTime);
            }
            break;
        case SELECTIVE:
            if (focus_item != items.end())
                focus_item->first->process(outputBuffer, inputBuffer, nBufferFrames, 0);
            break;
        default:
            break;
    }
}

void Rack::draw(GFXcanvas1 * screen) {
    if (focus_item != items.end())
        focus_item->first->draw(screen);
}

void Rack::add(AMG *item) {
    items.push_back({item, MIDISTATUS::DONE});
    focus_item = items.begin();

    if (racktype == PARALLEL) {
        if (outbuffer) free(outbuffer);
        outbuffer = new float[items.size() * BUF_SIZE * 2];
    }
}

Rack *Rack::dive_in() {
    if (dynamic_cast<Rack*>(focus_item->first) != nullptr)
        return dynamic_cast<Rack*>(focus_item->first);
    return this;
}

Rack *Rack::dive_out() {
    if (parent) return parent;
    return this;
}

Rack * Rack::dive_prev() {
    if (focus_item != items.begin()) focus_item --;
    else focus_item = items.end() - 1;
    return this;
}

Rack * Rack::dive_next() {
    if (focus_item + 1 != items.end()) focus_item ++;
    else focus_item = items.begin();
    return this;
}

void Rack::attach(Rack *parent_) {
    parent = parent_;
}

void Rack::set_focus_by_index(int i) {
    if (items.begin() + i < items.end()) focus_item = items.begin() + i;
}

AMG *Rack::get_focus() {
    return focus_item->first;
}

int Rack::get_focus_index() {
    return focus_item - items.begin();
}