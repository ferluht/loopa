//
// Created by ferluht on 07/01/2024.
//

#pragma once

#include <stdlib.h>

extern uint8_t SCREEN_IDX;

enum SCREENS {
    LOOP_VIEW=0,
    TAPE_VIEW=1,
    TRACK_VIEW=2,
    TRACK_EFFECTS_MIDI=3,
    TRACK_EFFECTS_AUDIO=4,
    MASTER_EFFECTS_MIDI=5,
    MASTER_EFFECTS_AUDIO=6,
    MIXER=7,
    RECORDER=8,
    PROJECT=9,
    FULLSCREEN,
    MAX_SCREENS=12
};
