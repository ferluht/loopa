//
// Created by ferluht on 20/07/2022.
//

#include "Scale.h"

Scale::Scale() : MIDIEffect("SCALE") {
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
            if ((selected_scale - scales.begin()) + 1 < scales.size()) selected_scale += 1;
            else selected_scale = scales.begin();
        } else {
            if ((selected_scale - scales.begin()) - 1 > -1) selected_scale -= 1;
            else selected_scale = scales.end() - 1;
        }

        update_note_map();

        return MIDISTATUS::DONE;
    });

    addMIDIHandler(MIDI::GENERAL::CC_HEADER, CC_E1, [this](MData &cmd) -> MIDISTATUS {
        int inc = cmd.data2 - 64;

        if (inc > 0) {
//            if ((selected_root - roots.begin()) + inc < roots.size()) selected_root += inc;
//            else selected_root = roots.begin();
            GLOBAL_SPEED += 1 / 100.0;
        } else {
//            if ((selected_root - roots.begin()) + inc > -1) selected_root += inc;
//            else selected_root = roots.end() - 1;
            GLOBAL_SPEED -= 1 / 100.0;
        }
        return MIDISTATUS::DONE;
    });
}

void Scale::update_note_map() {
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

void Scale::draw(GFXcanvas1 * screen) {
    screen->setCursor(4, 17);
    screen->print("SCALE");
    screen->setCursor(28, 17);
    std::string speed = std::to_string(GLOBAL_SPEED);
    std::string scale = selected_scale->first;
    screen->print((speed + " " + scale).c_str());
}

const char *Scale::getRoot() {
    return (*selected_root).c_str();
}

const char *Scale::getScale() {
    return selected_scale->first;
}