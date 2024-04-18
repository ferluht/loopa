//
// Created by ferluht on 07/01/2024.
//

#pragma once

#include <stdlib.h>

class Screened {

public:

    Screened() {
        setScreen(LOOPERS);
    }

    enum SCREENS {
        LOOPERS=0,
        TAPES,
        EFFECTS,
        MIXER,
        DEVICE,
        RECORDER,
        FULLSCREEN,
        MAX_SCREENS=12
    };

    uint8_t active_screen_idx;

    void setScreen(uint8_t screen_idx) {
        active_screen_idx = screen_idx;
    }
};
