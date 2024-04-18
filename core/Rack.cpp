//
// Created by ferluht on 28/07/2022.
//

#include "Rack.h"

void Rack::midiIn(MData &cmd, Sync & sync) {
    switch (racktype) {
        case PARALLEL:
            for (auto it = items.begin(); it < items.end(); it++) {
                MData ccmd = cmd;
                (*it)->midiIn(ccmd, sync);
            }
            break;
        case SEQUENTIAL:
            items.front()->midiIn(cmd, sync);
//            for (auto it = items.begin(); it < items.end(); it++) {
//                MData ccmd = cmd;
//                (*it)->midiIn(ccmd, sync);
//            }
            break;
        case SELECTIVE:
            if (focus_item != items.end())
                (*focus_item)->midiIn(cmd, sync);
            break;
        default:
            break;
    }
}

void Rack::midiOut(std::deque<MData> &q, Sync & sync) {
    switch (racktype) {
        case PARALLEL:
            for (auto it = items.begin(); it < items.end(); it++) {
                (*it)->midiOut(q, sync);
            }
            break;
        case SEQUENTIAL:
            for (auto it = items.begin(); it < items.end(); it++) {
                while (!q.empty()) {
                    MData cmd = q.front();
                    (*it)->midiIn(cmd, sync);
                    q.pop_front();
                }
                (*it)->midiOut(q, sync);
            }
            break;
        case SELECTIVE:
            if (focus_item != items.end())
                (*focus_item)->midiOut(q, sync);
            break;
        default:
            break;
    }
}

void Rack::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, Sync & sync) {
    int c = 0;
    switch (racktype) {
        case PARALLEL:
            for (unsigned int i = 0; i < 2 * nBufferFrames; i++) emptybuffer[i] = 0;

            for (auto it = items.begin(); it < items.end(); it++) {
                if (it == focus_item)
                    (*it)->process(&outbuffer[(it - items.begin()) * BUF_SIZE * 2], inputBuffer, nBufferFrames, sync);
                else
                    (*it)->process(&outbuffer[(it - items.begin()) * BUF_SIZE * 2], emptybuffer, nBufferFrames, sync);
            }

            for (unsigned int k = 0; k < items.size(); k ++)
                for (unsigned int i = 0; i < 2 * nBufferFrames; i ++)
                    outputBuffer[i] += outbuffer[k * BUF_SIZE * 2 + i];
            break;
        case SEQUENTIAL:
            for (auto it = items.begin(); it < items.end(); it++) {
                if (dynamic_cast<MIDIEffect*>(*it) != nullptr) continue;
                if (c != 0) {
                    for (unsigned int i = 0; i < 2 * nBufferFrames; i++) {
                        inputBuffer[i] = outputBuffer[i];
                        outputBuffer[i] = 0;
                    }
                }
                (*it)->process(outputBuffer, inputBuffer, nBufferFrames, sync);
                c++;
            }
            if (c == 0) {
                for (unsigned int i = 0; i < 2 * nBufferFrames; i++) {
                    outputBuffer[i] = inputBuffer[i];
                }
            }
            break;
        case SELECTIVE:
            if (inputBuffer[0] == 1) std::cout << getName() << "\n";
            if (focus_item != items.end())
                (*focus_item)->process(outputBuffer, inputBuffer, nBufferFrames, sync);
            break;
        default:
            break;
    }
}

void Rack::add(AMG *item) {
    items.push_back(item);
    focus_item = items.begin();

    if (racktype == PARALLEL) {
        if (outbuffer) free(outbuffer);
        outbuffer = new float[items.size() * BUF_SIZE * 2];
    }
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
    return *focus_item;
}

int Rack::get_focus_index() {
    return focus_item - items.begin();
}