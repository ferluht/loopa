//
// Created by ferluht on 20/07/2022.
//

#pragma once

#include <AMG.h>

class Scale : public AMG {

    std::vector<std::pair<const char *, std::vector<int>>> scales = {
            {"MIXOLYDIAN", {0, 2, 4, 5, 7, 9, 10}},
            {"CHROMATIC", {0, 1, 2, 3,4, 5, 6,7, 8,9, 10,11}},
            {"MAJOR", {0, 2, 4, 5, 7, 9, 11}},
            {"MINOR", {0, 2, 3, 5, 7, 8, 10}},
            {"HARM MINOR", {0, 2, 3, 5, 7, 8, 11}},
    };

    std::vector<std::pair<const char *, std::vector<int>>>::iterator selected_scale;

public:
    Scale() : AMG("SCALE") {
        selected_scale = scales.begin();
    }

    MIDISTATUS midiIn(MData &cmd) override;

    void draw(GFXcanvas1 * screen) override;
};
