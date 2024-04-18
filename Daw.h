//
// Created by ferluht on 07/07/2022.
//

#pragma once

#include <cstdint>
#include <AudioEffects.h>
#include <MidiEffects.h>
#include <Instruments.h>
#include <Rack.h>
#include <unistd.h>
#include <chrono>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <Sync.h>
#include <Tape/NewTape.h>
#include <Utils.h>
#include <Adafruit-GFX-offscreen/Fonts/Picopixel.h>

#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )

class DAW : public AMG {

    std::chrono::time_point<std::chrono::steady_clock> screenchange_time;
    int previous_screen;
    bool screen_toggle = false;
    int recording_blink_counter = 0;

    Rack * master_midi_fx;
    Rack * master_audio_fx;
    Scale * master_scale;

    WavFile<float> wf;
    StreamRecorder<float> sr;
    bool recording_master = false;

    Sync sync;

    const int n_tapes = 4;
    int octave = 0;

    float enc1_scale = 1, enc2_scale = 1;
    bool line_in = false;

public:

    DAW (GFXcanvas1 * screen_);

    void process(float *outputBuffer, float * inputBuffer, unsigned int nBufferFrames);

    void midiIn(MData& cmd);

private:

    void initHardwareControls();
    void initControlMappings();
    void initMidiHandlers();
    void initScreens();

    GFXcanvas1 * screen;

    std::mutex midiLock;
    std::deque<MData> midiInQueue;
    std::deque<MData> midiOutQueue;
    void midiOut(std::deque<MData> &q, Sync & sync) override;

    static Rack * spawnTracksRack();
    static Rack * spawnTapeRack(int n);
    static Rack * spawnSingleTrack();
    static Rack * spawnSingleTrack(const char * name, int i, int j, int k);
    static Rack * spawnMidiFXRack();
    static Rack * spawnInstrumentRack();
    static Rack * spawnAudioFXRack();


    inline AMG * getCurrentTrack() {return tracks->get_focus();}
    inline AMG * getCurrentTrackInstrument() {return ((Rack*)((Rack*)getCurrentTrack())->get_item(1))->get_focus();}
    inline Rack * getCurrentTrackMIDIFXRack() {return (Rack*)((Rack*)getCurrentTrack())->get_item(0);}
    inline Rack * getCurrentTrackAudioFXRack() {return (Rack*)((Rack*)getCurrentTrack())->get_item(2);}

    Rack * tracks;
    Tape * tapes;

    MIDIMap * midiMap;
};

