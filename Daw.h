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
#include <tinyxml2.h>
#include <fstream>
#include <utility>

#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )

class OnboardingNode {

public:

    int state = 0;
    bool block = true;
    int time_delay = 0;
    int draw_calls_counter = 0;
    int flash_period = 40;
    float on_percentage = 1;
    std::function<bool(MData&, Sync&, int& state)> condition;
    std::function<void(GFXcanvas1 * screen, int& state)> drawer;

    OnboardingNode(std::function<bool(MData&, Sync&, int& state)> condition,
                   std::function<void(GFXcanvas1 * screen, int& state)> drawer,
                   bool block, int time_delay);

    static OnboardingNode createBannerQuestion(std::vector<std::string> lines, int time_delay=15);
    static OnboardingNode createBannerExitAny(std::vector<std::string> lines, int time_delay=15);
    static OnboardingNode createBannerExitTime(std::vector<std::string> lines, int time_delay=3, int time_exit=15);
    static OnboardingNode createBannerExitStatus(std::vector<std::string> lines, uint8_t status, bool block=true, int time_delay=15);
    static OnboardingNode createEmptyExitStatus(uint8_t status, bool block=false, int time_delay=0);
    static OnboardingNode createEmptyExitStatusAndData(uint8_t status, uint8_t data1, bool block=false, int time_delay=0);
    static OnboardingNode createBannerExitStatusAndData(std::vector<std::string> lines, uint8_t status, uint8_t data1, bool block=true, int time_delay=15);
    static OnboardingNode createBannerExitStatusStartStop(std::vector<std::string> lines, uint8_t status_start, uint8_t status_stop, bool block=true, int time_delay=15);
};

class Onboarding : public AMG {

public:

    Onboarding();

    void draw(GFXcanvas1 * screen);
    void midiIn(MData& cmd, Sync & sync);

    void reset();

    std::vector<OnboardingNode> script;
    std::vector<OnboardingNode>::iterator current_pos;

    int draw_counter = 0;
};

class Keyboard : public AMG {

public:

    Keyboard();

    void enable(std::string title_, std::string initial_entry_, std::function<void(std::string)> onclose_);

    std::function<void(std::string)> onclose;
    bool isactive = false;
    int start_letter = 0;
    std::string title, entry;
};

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

    int last_midi_time = 0;

    Sync sync;

    const int n_tapes = 4;
    int octave = 0;

    bool reloaded = true;
    std::mutex reload_mutex;

    float enc1_scale = 1, enc2_scale = 1;
    bool line_in = false;

    enum DAWSTATE {
        NORMAL,
        SAVING,
        LOADING,
        PAUSED
    } dawState;

    const std::string wlan_cfg_path = "/etc/wpa_supplicant/wpa_supplicant-wlan0.conf";
    std::string wlan_cfg;
    bool simulated_cfg = false;
    void read_wlan_settings();
    void write_wlan_settings();
    void get_ssid(std::string & ssid);
    void get_pwd(std::string & pwd);
    void set_ssid(std::string & ssid);
    void set_pwd(std::string & pwd);

public:

    DAW (GFXcanvas1 * screen_);

    void process(float *outputBuffer, float * inputBuffer, unsigned int nBufferFrames);

    void midiIn(MData& cmd);

    void save();
    void load();
    void draw(GFXcanvas1 * screen);

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
    inline AMG * getCurrentTrackInstrument() {return ((Rack*)getCurrentTrack())->get_item(1);}
    inline Rack * getCurrentTrackMIDIFXRack() {return (Rack*)((Rack*)getCurrentTrack())->get_item(0);}
    inline Rack * getCurrentTrackAudioFXRack() {return (Rack*)((Rack*)getCurrentTrack())->get_item(2);}

    Rack * tracks;
    Tape * tapes;

    MIDIMap * midiMap;

    Keyboard kbd;
    Onboarding obd;

    std::mutex dawStateMutex;
};

