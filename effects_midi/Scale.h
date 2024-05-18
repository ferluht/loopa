//
// Created by ferluht on 20/07/2022.
//

#pragma once

#include <AMG.h>

class Scale : public MIDIEffect {

    std::vector<std::pair<const char *, std::vector<int>>> scales = {
            {"MIXOLYDIAN", {0, 2, 4, 5, 7, 9, 10}},
            {"CHROMATIC", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}},
            {"MAJOR", {0, 2, 4, 5, 7, 9, 11}},
            {"MINOR", {0, 2, 3, 5, 7, 8, 10}},
            {"HARM MINOR", {0, 2, 3, 5, 7, 8, 11}},
            {"PHRYGIAN", {0, 1, 3, 5, 7, 8, 10}},
    };

    std::vector<std::string> roots = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    std::vector<std::string>::iterator selected_root;
    std::vector<int> note_map;
    std::vector<std::pair<const char *, std::vector<int>>>::iterator selected_scale;
    void update_note_map();

public:
    static DeviceFactory* create() { return new Scale(); }
    Scale();
    void midiOut(std::deque<MData> &q, Sync & sync) override;
};
