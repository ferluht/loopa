//
// Created by ferluht on 13/01/2024.
//

#pragma once

#include <set>
#include <Effect.h>

class Arpeggiator : public MIDIEffect {

    Parameter * arp_time;
    Parameter * sync_switch;
    float time;
    std::set<uint8_t> pressed_notes;
    uint8_t last_note = 0;

public:

    Arpeggiator();

    void midiOut(std::deque<MData> &q, Sync & sync) override;

private:
    uint8_t arp_step(std::set<uint8_t>::iterator it);
};

