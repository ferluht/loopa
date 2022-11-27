//
// Created by ferluht on 20/07/2022.
//

#include "Scale.h"

void Scale::midiIn(MData &cmd) {
    int scale[] = {0, 2, 4, 5, 7, 9, 10};
//    int scale[] = {0, 2, 3, 6, 7, 8, 11};
    if (cmd.status == NOTEON_HEADER || cmd.status == NOTEOFF_HEADER) {
        cmd.data1 = scale[(cmd.data1 - 36) % 7] + (int)((cmd.data1 - 36) / 7) * 12;
    }
}

void Scale::draw(GFXcanvas1 * screen) {
    screen->setCursor(4, 16);
    screen->setTextSize(1);
    screen->print("C MIXOLYDIAN");
}