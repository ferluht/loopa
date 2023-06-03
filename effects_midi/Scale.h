//
// Created by ferluht on 20/07/2022.
//

#pragma once

#include <Effect.h>

class Scale : public MIDIEffect {

    std::vector<std::pair<const char *, std::vector<int>>> scales = {
            {"MIXOLYDIAN", {0, 2, 4, 5, 7, 9, 10}},
            {"CHROMATIC", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}},
            {"MAJOR", {0, 2, 4, 5, 7, 9, 11}},
            {"MINOR", {0, 2, 3, 5, 7, 8, 10}},
            {"HARM MINOR", {0, 2, 3, 5, 7, 8, 11}},
    };

    std::vector<std::string> roots = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    std::vector<std::string>::iterator selected_root;

    std::vector<int> note_map;

    std::vector<std::pair<const char *, std::vector<int>>>::iterator selected_scale;

public:
    Scale() : MIDIEffect("SCALE") {
        selected_scale = scales.begin();
        selected_root = roots.begin();

        for (int i = 0; i < 12; i ++) note_map.push_back(i);
        update_note_map();

        addMIDIHandler({MIDI::GENERAL::NOTEON_HEADER, MIDI::GENERAL::NOTEOFF_HEADER}, [this](MData &cmd) -> MIDISTATUS {
//            std::vector<int> * scale = &selected_scale->second;

            cmd.data1 = (int)(cmd.data1 / 12) * 12 + note_map[cmd.data1 % 12] + selected_root - roots.begin();

//            cmd.data1 = (*scale)[(cmd.data1 - 36) % scale->size()] + (int)((cmd.data1 - 36) / scale->size()) * 12;
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, CC_E2, [this](MData &cmd) -> MIDISTATUS {
            int inc = cmd.data2 - 64;

            if (inc > 0) {
                if ((selected_scale - scales.begin()) + inc < scales.size()) selected_scale += inc;
                else selected_scale = scales.begin();
            } else {
                if ((selected_scale - scales.begin()) + inc > -1) selected_scale += inc;
                else selected_scale = scales.end() - 1;
            }

            update_note_map();

            return MIDISTATUS::DONE;
        });

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, CC_E1, [this](MData &cmd) -> MIDISTATUS {
            int inc = cmd.data2 - 64;

            if (inc > 0) {
                if ((selected_root - roots.begin()) + inc < roots.size()) selected_root += inc;
                else selected_root = roots.begin();
            } else {
                if ((selected_root - roots.begin()) + inc > -1) selected_root += inc;
                else selected_root = roots.end() - 1;
            }
            return MIDISTATUS::DONE;
        });
    }

    void update_note_map() {
        if (selected_scale->second.size() == 12) {
            for (int i = 0; i < 12; i ++) note_map[i] = selected_scale->second[i];
        } else {
            note_map[0] = selected_scale->second[0];
            note_map[1] = selected_scale->second[0];
            note_map[2] = selected_scale->second[1];
            note_map[3] = selected_scale->second[1];
            note_map[4] = selected_scale->second[2];
            note_map[5] = selected_scale->second[3];
            note_map[6] = selected_scale->second[3];
            note_map[7] = selected_scale->second[4];
            note_map[8] = selected_scale->second[4];
            note_map[9] = selected_scale->second[5];
            note_map[10] = selected_scale->second[5];
            note_map[11] = selected_scale->second[6];
        }
    }

    void draw(GFXcanvas1 * screen) override;
};
