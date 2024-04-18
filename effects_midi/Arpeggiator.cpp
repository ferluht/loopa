//
// Created by ferluht on 13/01/2024.
//

#include "Arpeggiator.h"

Arpeggiator::Arpeggiator() : MIDIEffect("ARP") {
    enable(false);

    arp_time = addParameter("RATE", 0.1, 16, 7, 0.1);
    sync_switch = addParameter("SYNC", {"ON", "OFF"}, 1);

    addParameter("-", {"ON", "OFF"}, 1);

    addMIDIHandler({}, {MIDI::GENERAL::NOTEON_HEADER, MIDI::GENERAL::NOTEOFF_HEADER}, {}, [this](MData &cmd, Sync &sync) -> void {
        switch (cmd.status) {
            case MIDI::GENERAL::NOTEON_HEADER:
                pressed_notes.insert(cmd.data1);
                break;
            case MIDI::GENERAL::NOTEOFF_HEADER:
                pressed_notes.erase(cmd.data1);
                break;
            default:
                break;
        }
    });

    last_note = 255;
}

void Arpeggiator::midiOut(std::deque<MData> &q, Sync & sync) {

    if (ison->getStringVal() == "FALSE") {
        M::midiOut(q, sync);
        return;
    }

    if (pressed_notes.empty()) {
        if (last_note < 255) {
            MData cmd = {0, MIDI::GENERAL::NOTEOFF_HEADER, last_note, 0};
            q.emplace_back(cmd);
            last_note = 255;
        }
        return;
    }
    time += BUF_SIZE;
    if (time > 2 && last_note < 255) {
        MData cmd = {0, MIDI::GENERAL::NOTEOFF_HEADER, last_note, 0};
        q.emplace_back(cmd);
    }
    if ((sync_switch->getStringVal() == "OFF" && time > SAMPLERATE * arp_time->value) ||
        (sync_switch->getStringVal() == "ON" && time > SAMPLERATE * (60.0 / sync.getBPM() * 4 / 48.0) * (int)(arp_time->value / 0.5))) {
        time -= SAMPLERATE * (60.0 / sync.getBPM() * 4 / 48.0) * (int)(arp_time->value / 0.5);
        auto it = pressed_notes.find(last_note);
        if (it == pressed_notes.end()) it = pressed_notes.begin();
        last_note = arp_step(it);
        MData cmd = {0, MIDI::GENERAL::NOTEON_HEADER, last_note, 120};
        q.emplace_back(cmd);
    }
}

uint8_t Arpeggiator::arp_step(std::set<uint8_t>::iterator it) {
    it++;
    if (it == pressed_notes.end()) it = pressed_notes.begin();
    return *it;
}