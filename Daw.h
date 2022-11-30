//
// Created by ferluht on 07/07/2022.
//

#pragma once

#include <cstdint>
#include <AMG.h>
#include <Instruments.h>
#include <AudioEffects.h>
#include <MidiEffects.h>
#include <Ui.h>
#include <unistd.h>
#include <chrono>
#include <mutex>

#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )

class DAW : public AMG{

public:

    DAW(GFXcanvas1 * screen_) : DAW(3, 4, screen_) {};

    DAW (int n_tracks, int n_tapes, GFXcanvas1 * screen_) : AMG("DAW") {
        screen = screen_;

        midiInterface = new MIDIInterface();
        midiInterface->addHardwareControl("CTRL", CC_HEADER, 100);
        midiInterface->addHardwareControl("SHIFT", CC_HEADER, 101);

        midiInterface->addHardwareControl("P1", CC_HEADER, 110);
        midiInterface->addHardwareControl("P2", CC_HEADER, 111);
        midiInterface->addHardwareControl("P3", CC_HEADER, 112);
        midiInterface->addHardwareControl("P4", CC_HEADER, 113);

        midiInterface->addHardwareControl("K1", CC_HEADER, 60);
        midiInterface->addHardwareControl("K2", CC_HEADER, 61);
        midiInterface->addHardwareControl("K3", CC_HEADER, 62);
        midiInterface->addHardwareControl("K4", CC_HEADER, 63);
        midiInterface->addHardwareControl("K5", CC_HEADER, 64);
        midiInterface->addHardwareControl("K6", CC_HEADER, 65);
        midiInterface->addHardwareControl("K7", CC_HEADER, 66);

        midiInterface->addMapping({"SHIFT", "K1"}, DAW_CTRL_HEADER, MIDICC::DAW_UP);
        midiInterface->addMapping({"SHIFT", "K2"}, DAW_CTRL_HEADER, MIDICC::DAW_DOWN);
        midiInterface->addMapping({"SHIFT", "K3"}, DAW_CTRL_HEADER, MIDICC::DAW_RIGHT);
        midiInterface->addMapping({"SHIFT", "K4"}, DAW_CTRL_HEADER, MIDICC::DAW_LEFT);

        midiInterface->addMapping({"P1"}, CC_HEADER + 0, MIDICC::TAPE_TRIG);
        midiInterface->addMapping({"SHIFT", "P1"}, CC_HEADER + 0, MIDICC::TAPE_CLEAR);
        midiInterface->addMapping({"CTRL", "P1"}, CC_HEADER + 0, MIDICC::TAPE_DOUBLE);
        midiInterface->addMapping({"SHIFT", "CTRL", "P1"}, CC_HEADER + 0, MIDICC::TAPE_STOP);

        midiInterface->addMapping({"P2"}, CC_HEADER + 1, MIDICC::TAPE_TRIG);
        midiInterface->addMapping({"SHIFT", "P2"}, CC_HEADER + 1, MIDICC::TAPE_CLEAR);
        midiInterface->addMapping({"CTRL", "P2"}, CC_HEADER + 1, MIDICC::TAPE_DOUBLE);
        midiInterface->addMapping({"SHIFT", "CTRL", "P2"}, CC_HEADER + 1, MIDICC::TAPE_STOP);

        midiInterface->addMapping({"P3"}, CC_HEADER + 2, MIDICC::TAPE_TRIG);
        midiInterface->addMapping({"SHIFT", "P3"}, CC_HEADER + 2, MIDICC::TAPE_CLEAR);
        midiInterface->addMapping({"CTRL", "P3"}, CC_HEADER + 2, MIDICC::TAPE_DOUBLE);
        midiInterface->addMapping({"SHIFT", "CTRL", "P3"}, CC_HEADER + 2, MIDICC::TAPE_STOP);

        midiInterface->addMapping({"P4"}, CC_HEADER + 3, MIDICC::TAPE_TRIG);
        midiInterface->addMapping({"SHIFT", "P4"}, CC_HEADER + 3, MIDICC::TAPE_CLEAR);
        midiInterface->addMapping({"CTRL", "P4"}, CC_HEADER + 3, MIDICC::TAPE_DOUBLE);
        midiInterface->addMapping({"SHIFT", "CTRL", "P4"}, CC_HEADER + 3, MIDICC::TAPE_STOP);

        tracks = spawnTracksRack(n_tracks);
        tapes = new TapeRack();

        focus_rack = tracks;

        dawMidiStatus = MIDISTATUS::DONE;
    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;

    MIDISTATUS midiIn(MData& cmd) override;

    void draw(GFXcanvas1 * screen) override;

private:

    GFXcanvas1 * screen;

    std::mutex midiLock;
    std::deque<MData> midiQueue;

    static Rack * spawnTracksRack(int n);
    static Rack * spawnTapeRack(int n);
    static Rack * spawnSingleTrack();
    static Rack * spawnSingleTrack(const char * name, int i, int j, int k);
    static Rack * spawnMidiRack();
    static Rack * spawnInstrumentRack();
    static Rack * spawnEffectRack();

    Rack * tracks;
    TapeRack * tapes;
    Rack * focus_rack;

    MIDISTATUS dawMidiStatus;

    MIDIInterface * midiInterface;

    float avg = 0;
    float avg_env = 0;
    inline float envelope(float sample, float w, float w_env) {
        avg = w * sample + (1 - w) * avg;
        float i = std::abs(sample - avg);
        avg_env = w_env * i + (1 - w_env) * avg_env;
        return avg_env;
    }
};

