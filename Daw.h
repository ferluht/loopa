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
#include <LoopMatrix/LoopMatrix.h>

#define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )

class DAW : public AMG{

    enum FSCREEN {
        MAIN,
        LOOP,
        MIX
    };

    std::chrono::time_point<std::chrono::steady_clock> screenchange_time;
    int previous_screen;
    int current_screen;
    bool screen_toggle = false;

    Rack * master_fx;

public:

    DAW(GFXcanvas1 * screen_) : DAW(3, 4, screen_) {};

    DAW (int n_tracks, int n_tapes, GFXcanvas1 * screen_) : AMG("DAW") {
        screen = screen_;

        midiMap = new MIDIMap();
        midiMap->addHardwareControl("CTRL", MIDI::GENERAL::CC_HEADER, 100);
        midiMap->addHardwareControl("SHIFT", MIDI::GENERAL::CC_HEADER, 101);
        midiMap->addHardwareControl("COPY", MIDI::GENERAL::CC_HEADER, 66);

        midiMap->addHardwareControl("P0", MIDI::GENERAL::CC_HEADER, 110);
        midiMap->addHardwareControl("P1", MIDI::GENERAL::CC_HEADER, 111);
        midiMap->addHardwareControl("P2", MIDI::GENERAL::CC_HEADER, 112);
        midiMap->addHardwareControl("P3", MIDI::GENERAL::CC_HEADER, 113);

        midiMap->addHardwareControl("K1", MIDI::GENERAL::CC_HEADER, 60);
        midiMap->addHardwareControl("K2", MIDI::GENERAL::CC_HEADER, 61);
        midiMap->addHardwareControl("K3", MIDI::GENERAL::CC_HEADER, 62);
        midiMap->addHardwareControl("K4", MIDI::GENERAL::CC_HEADER, 63);

        midiMap->addHardwareControl("ALT", MIDI::GENERAL::CC_HEADER, 64);
        midiMap->addHardwareControl("LOOP", MIDI::GENERAL::CC_HEADER, 65);
//        midiMap->addHardwareControl("SAVE", MIDI::GENERAL::CC_HEADER, 66);

        midiMap->addMapping({"SHIFT", "K1"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::LEFT);
        midiMap->addMapping({"SHIFT", "K2"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::DOWN);
        midiMap->addMapping({"SHIFT", "CTRL", "K2"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::DOWNDOWN);
        midiMap->addMapping({"SHIFT", "K3"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::RIGHT);
        midiMap->addMapping({"SHIFT", "K4"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::SAVE);
        midiMap->addMapping({"SHIFT", "CTRL", "K4"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::UPUP);

        midiMap->addMapping({"ALT"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::SCREEN::ALT_PARAMS);
        midiMap->addMapping({"SHIFT", "LOOP"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::SCREEN::LOOP_SCREEN);

        for (int i = 0; i < n_tapes; i ++) {
            auto kn = "P" + std::to_string(i);
            midiMap->addMapping({kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::TAPE::TRIG);
            midiMap->addMapping({"SHIFT", kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::TAPE::CLEAR);
            midiMap->addMapping({"CTRL", kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::TAPE::DOUBLE);
            midiMap->addMapping({"SHIFT", "CTRL", kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::TAPE::STOP);
            midiMap->addMapping({"COPY", kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::LOOPMATRIX::COPY);
            midiMap->addMapping({"LOOP", kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::LOOPMATRIX::SELECT_SCENE);
            midiMap->addMapping({"LOOP", "COPY", kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::LOOPMATRIX::COPY_SCENE);
        }

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::LEFT, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0) {
                if (current_screen == FSCREEN::MAIN) {
                    switch (focus_rack_depth) {
                        case 1:
                            this->focus_rack = (Rack *) this->focus_rack->dive_out()->get_item(1);
                            focus_rack_depth = 0;
                            break;
                        case 2:
                            this->focus_rack = (Rack *) this->focus_rack->dive_out()->get_item(0);
                            focus_rack_depth = 1;
                            break;
                        default:
                            break;
                    }
                } else if (current_screen == FSCREEN::LOOP) {
                    master_fx->dive_prev();
                }
            }
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::DOWN, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0) {
                switch (focus_rack_depth) {
                    case 0:
                        tracks->dive_next();
                        this->focus_rack = (Rack *)(((Rack *) tracks->get_focus())->get_item(1));
                        break;
                    case 1:
                    case 2:
                        this->focus_rack = (Rack*) this->focus_rack->dive_next();
                        break;
                    default:
                        break;
                }
            }
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::DOWNDOWN, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0) {
                switch (focus_rack_depth) {
                    case 0:
                        ((Rack *)(((Rack *) tracks->get_focus())->get_item(1)))->dive_prev();
                        this->focus_rack = (Rack *)(((Rack *) tracks->get_focus())->get_item(1));
                        break;
                    default:
                        break;
                }
            }
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::RIGHT, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0) {
                if (current_screen == FSCREEN::MAIN) {
                    switch (focus_rack_depth) {
                        case 0:
                            this->focus_rack = (Rack *) this->focus_rack->dive_out()->get_item(0);
                            focus_rack_depth = 1;
                            break;
                        case 1:
                            this->focus_rack = (Rack *) this->focus_rack->dive_out()->get_item(2);
                            focus_rack_depth = 2;
                            break;
                        default:
                            break;
                    }
                } else if (current_screen == FSCREEN::LOOP) {
                    master_fx->dive_next();
                }
            }
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::UP, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0) {
                switch (focus_rack_depth) {
                    case 0:
                        tracks->dive_prev();
                        this->focus_rack = (Rack *)(((Rack *) tracks->get_focus())->get_item(1));
                        break;
                    case 1:
                    case 2:
                        this->focus_rack = (Rack*) this->focus_rack->dive_prev();
                        break;
                    default:
                        break;
                }
            }
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::UPUP, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0) {
                switch (focus_rack_depth) {
                    case 0:
                        ((Rack *)(((Rack *) tracks->get_focus())->get_item(1)))->dive_next();
                        this->focus_rack = (Rack *)(((Rack *) tracks->get_focus())->get_item(1));
                        break;
                    default:
                        break;
                }
            }
            return MIDISTATUS::DONE;
        });

        addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::SCREEN::LOOP_SCREEN, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0) {
                if (!screen_toggle) {
                    previous_screen = current_screen;
                    if (current_screen != FSCREEN::LOOP) {
                        current_screen = FSCREEN::LOOP;
                        tapes->select_screen(1);
                    } else {
                        current_screen = FSCREEN::MAIN;
                        tapes->select_screen(0);
                    }
                    screenchange_time = std::chrono::steady_clock::now();
                }
                screen_toggle = true;
            } else {
                auto timedelta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - screenchange_time).count();
                if (timedelta > 300) {
                    current_screen = previous_screen;
                    if (current_screen == FSCREEN::MAIN) tapes->select_screen(0);
                    else if (current_screen == FSCREEN::LOOP) tapes->select_screen(1);
                }
                screen_toggle = false;
            }
            return MIDISTATUS::DONE;
        });

        tracks = spawnTracksRack(n_tracks);
        tapes = new LoopMatrix<2,2>();

        master_fx = new Rack("MASTER FX", Rack::SELECTIVE);
        master_fx->add(new Scale());
        master_fx->add(tapes);

        tapes->addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::SAVE, [this](MData &cmd) -> MIDISTATUS {
            if (cmd.data2 > 0) {
                if (tapes->save()) return MIDISTATUS::DONE;
                else return MIDISTATUS::WAITING;
            }
            return MIDISTATUS::DONE;
        });

        focus_rack = (Rack*)((Rack*)tracks->get_focus())->get_item(1);
        focus_rack_depth = 0;

        dawMidiStatus = MIDISTATUS::DONE;
    }

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;

    MIDISTATUS midiIn(MData& cmd) override;

    void draw(GFXcanvas1 * screen) override;

private:

//    void drawMixScreen(GFXcanvas1 * screen);
    void drawMainScreen(GFXcanvas1 * screen);
    void drawLoopScreen(GFXcanvas1 * screen);

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
    LoopMatrix<2,2> * tapes;
    Rack * focus_rack;
    int focus_rack_depth = 0;

    MIDISTATUS dawMidiStatus;

    MIDIMap * midiMap;

    float avg = 0;
    float avg_env = 0;
    inline float envelope(float sample, float w, float w_env) {
        avg = w * sample + (1 - w) * avg;
        float i = std::abs(sample - avg);
        avg_env = w_env * i + (1 - w_env) * avg_env;
        return avg_env;
    }
};

