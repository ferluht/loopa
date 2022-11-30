//
// Created by ferluht on 20/07/2022.
//

#include "Scale.h"

MIDISTATUS Scale::midiIn(MData &cmd) {
    if (cmd.status == NOTEON_HEADER || cmd.status == NOTEOFF_HEADER) {
        std::vector<int> * scale = &selected_scale->second;
        cmd.data1 = (*scale)[(cmd.data1 - 36) % scale->size()] + (int)((cmd.data1 - 36) / scale->size()) * 12;
    } else if (cmd.status == CC_HEADER) {
        if (cmd.data1 == CC_E1) {
            if (selected_scale + 1 != scales.end())
                selected_scale ++;
            else
                selected_scale = scales.begin();
        }
    }
    return MIDISTATUS::DONE;
}

void Scale::draw(GFXcanvas1 * screen) {
    screen->setCursor(4, 16);
    screen->setTextSize(1);
    screen->print(selected_scale->first);
}