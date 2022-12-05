//
// Created by ferluht on 20/07/2022.
//

#pragma once

#include <Effect.h>

class Scale : public MIDIEffect {

    std::vector<std::pair<const char *, std::vector<int>>> scales = {
            {"MIXOLYDIAN", {0, 2, 4, 5, 7, 9, 10}},
            {"CHROMATIC", {0, 1, 2, 3,4, 5, 6,7, 8,9, 10,11}},
            {"MAJOR", {0, 2, 4, 5, 7, 9, 11}},
            {"MINOR", {0, 2, 3, 5, 7, 8, 10}},
            {"HARM MINOR", {0, 2, 3, 5, 7, 8, 11}},
    };

    std::vector<std::pair<const char *, std::vector<int>>>::iterator selected_scale;

public:
    Scale() : MIDIEffect("SCALE") {
        selected_scale = scales.begin();

        addMIDIHandler({MIDI::GENERAL::NOTEON_HEADER, MIDI::GENERAL::NOTEOFF_HEADER}, [this](MData &cmd) -> MIDISTATUS {
            std::vector<int> * scale = &selected_scale->second;
            cmd.data1 = (*scale)[(cmd.data1 - 36) % scale->size()] + (int)((cmd.data1 - 36) / scale->size()) * 12;
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, CC_E1, [this](MData &cmd) -> MIDISTATUS {
            if (selected_scale + 1 != scales.end()) selected_scale++;
            else selected_scale = scales.begin();
            return MIDISTATUS::DONE;
        });
    }

    void draw(GFXcanvas1 * screen) override;
};
