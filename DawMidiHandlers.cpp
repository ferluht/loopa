//
// Created by ferluht on 06/01/2024.
//

#include "Daw.h"

void DAW::initHardwareControls() {
    midiMap = new MIDIMap();

    midiMap->addHardwareControl("CTRL", MIDI::GENERAL::CC_HEADER, 100);
    midiMap->addHardwareControl("PAGE", MIDI::GENERAL::CC_HEADER, 101);
    midiMap->addHardwareControl("SHFT", MIDI::GENERAL::CC_HEADER, 102);

    midiMap->addHardwareControl("O-", MIDI::GENERAL::CC_HEADER, 98);
    midiMap->addHardwareControl("O+", MIDI::GENERAL::CC_HEADER, 99);

    midiMap->addHardwareControl("E1", MIDI::GENERAL::CC_HEADER, CC_E1);
    midiMap->addHardwareControl("E2", MIDI::GENERAL::CC_HEADER, CC_E2);

    for (int i = 0; i < 4; i ++)
        midiMap->addHardwareControl("L" + std::to_string(i), MIDI::GENERAL::CC_HEADER, 110 + i);

    for (int i = 0; i < 12; i ++)
        midiMap->addHardwareControl("K" + std::to_string(i), MIDI::GENERAL::CC_HEADER, 60 + i);
}

void DAW::initControlMappings() {

    midiMap->addMapping({"CTRL"}, MIDI::GENERAL::CTRL_HEADER, MIDI::UI::PLAY_BUTTON);
    midiMap->addMapping({"CTRL", "PAGE"}, MIDI::GENERAL::CTRL_HEADER, MIDI::UI::REWIND_BACKWARD_BUTTON);
    midiMap->addMapping({"CTRL", "SHFT"}, MIDI::GENERAL::CTRL_HEADER, MIDI::UI::REWIND_FORWARD_BUTTON);
    midiMap->addMapping({"PAGE", "CTRL"}, MIDI::GENERAL::CTRL_HEADER, MIDI::UI::REC_BUTTON);
    midiMap->addMapping({"SHFT", "CTRL"}, MIDI::GENERAL::CTRL_HEADER, MIDI::UI::CLICK_BUTTON);

    midiMap->addMapping({"PAGE", "E1"}, MIDI::GENERAL::CC_HEADER, CC_E3);
    midiMap->addMapping({"PAGE", "E2"}, MIDI::GENERAL::CC_HEADER, CC_E4);

    for (int i = 0; i < 12; i ++) {
        midiMap->addMapping({"K" + std::to_string(i)}, MIDI::GENERAL::NOTE_HEADER, i);
        midiMap->addMapping({"SHFT", "K" + std::to_string(i)}, MIDI::GENERAL::SHFT_HEADER, i);
        midiMap->addMapping({"CTRL", "K" + std::to_string(i)}, MIDI::GENERAL::CTRL_HEADER, i);
        midiMap->addMapping({"PAGE", "K" + std::to_string(i)}, MIDI::GENERAL::PAGE_HEADER, i);
    }

    midiMap->addMapping({"PAGE", "O+"}, MIDI::GENERAL::BUTN_HEADER, MIDI::UI::DAW::DUMMY);
    midiMap->addMapping({"PAGE", "O-"}, MIDI::GENERAL::BUTN_HEADER, MIDI::UI::DAW::DUMMY);
    midiMap->addMapping({"PAGE", "O+", "O-"}, MIDI::GENERAL::BUTN_HEADER, MIDI::UI::DAW::REC);
    midiMap->addMapping({"PAGE", "O-", "O+"}, MIDI::GENERAL::BUTN_HEADER, MIDI::UI::DAW::REC);

    midiMap->addMapping({"SHFT", "O+"}, MIDI::GENERAL::BUTN_HEADER, MIDI::UI::DAW::UP);
    midiMap->addMapping({"SHFT", "O-"}, MIDI::GENERAL::BUTN_HEADER, MIDI::UI::DAW::DOWN);

    midiMap->addMapping({"O+"}, MIDI::GENERAL::BUTN_HEADER, 99);
    midiMap->addMapping({"O-"}, MIDI::GENERAL::BUTN_HEADER, 98);

//    midiMap->addMapping({"SHIFT", "K1"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::LEFT);
//    midiMap->addMapping({"SHIFT", "K2"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::DOWN);
//    midiMap->addMapping({"SHIFT", "CTRL", "K2"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::DOWNDOWN);
//    midiMap->addMapping({"SHIFT", "K3"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::RIGHT);
//    midiMap->addMapping({"SHIFT", "K4"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::SAVE);
//    midiMap->addMapping({"SHIFT", "CTRL", "K4"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::DAW::UPUP);
//
//    midiMap->addMapping({"ALT"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::SCREEN::ALT_PARAMS);
//    midiMap->addMapping({"SHIFT", "LOOP"}, MIDI::GENERAL::CC_HEADER, MIDI::UI::SCREEN::LOOP_SCREEN);
//
    for (int i = 0; i < n_tapes; i ++) {
        auto kn = "L" + std::to_string(i);
        midiMap->addMapping({kn}, MIDI::GENERAL::LOOP_HEADER + i, MIDI::UI::TAPE::TRIG);
        midiMap->addMapping({"SHFT", kn}, MIDI::GENERAL::LOOP_HEADER + i, MIDI::UI::TAPE::CLEAR);
        midiMap->addMapping({"CTRL", kn}, MIDI::GENERAL::LOOP_HEADER + i, MIDI::UI::TAPE::DOUBLE);
        midiMap->addMapping({"SHFT", "CTRL", kn}, MIDI::GENERAL::LOOP_HEADER + i, MIDI::UI::TAPE::STOP);
        for (int j = 0; j < 12; j ++)
            midiMap->addMapping({kn, "K" + std::to_string(j)}, MIDI::GENERAL::PATTERN_HEADER + i, j);
//        midiMap->addMapping({"COPY", kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::LOOPMATRIX::COPY);
//        midiMap->addMapping({"LOOP", kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::LOOPMATRIX::SELECT_SCENE);
//        midiMap->addMapping({"LOOP", "COPY", kn}, MIDI::GENERAL::CC_HEADER + i, MIDI::UI::LOOPMATRIX::COPY_SCENE);
    }
}

void DAW::initMidiHandlers() {

    // GENERAL HANDLERS

    addMIDIHandler({}, {MIDI::GENERAL::PAGE_HEADER}, {}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 > 0) {
            if (!screen_toggle) {
                previous_screen = SCREEN_IDX;
                SCREEN_IDX = cmd.data1 % SCREENS::MAX_SCREENS;
                screenchange_time = std::chrono::steady_clock::now();
            }
            screen_toggle = true;
        } else {
            auto timedelta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - screenchange_time).count();
            if (timedelta > 300) SCREEN_IDX = previous_screen;
            screen_toggle = false;
        }
    });

    addMIDIHandler({}, {MIDI::GENERAL::BUTN_HEADER}, {98, 99}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 == 0) return;
        if (cmd.data1 == 98) octave --;
        else octave ++;
        if (octave < -4) octave = -4;
        if (octave >  4) octave =  4;
    });

    addMIDIHandler({SCREENS::LOOP_VIEW, SCREENS::TAPE_VIEW},
                   {MIDI::GENERAL::BUTN_HEADER},
                   {MIDI::UI::DAW::REC}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 == 0) return;
        if (recording_master) {
            recording_master = false;
            sr.openFile("");
            sr.closeFile();
        } else {
            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);

            std::ostringstream oss;
            oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
            auto timestr = oss.str();

            std::string savedir = "../res/saved/" + timestr;

            mkdir(savedir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

            sr.openFile(savedir + "/master.wav");
            recording_master = true;
        }
    });

    addMIDIHandler(0, SCREENS::MAX_SCREENS,
                   MIDI::GENERAL::LOOP_HEADER, MIDI::GENERAL::LOOP_HEADER + n_tapes,
                   0, 128,
                   [this](MData &cmd, Sync &sync) -> void {
        if (SCREEN_IDX != SCREENS::TRACK_VIEW) tapes->midiIn(cmd, sync);
        else getCurrentTrackInstrument()->midiIn(cmd, sync);
    });

    addMIDIHandler(0, SCREENS::MAX_SCREENS,
                   MIDI::GENERAL::PATTERN_HEADER, MIDI::GENERAL::PATTERN_HEADER + n_tapes,
                   0, 128,
                   [this](MData &cmd, Sync &sync) -> void {
        if (SCREEN_IDX != SCREENS::TRACK_VIEW) tapes->midiIn(cmd, sync);
    });

    addMIDIHandler({SCREENS::LOOP_VIEW, SCREENS::TAPE_VIEW, SCREENS::TRACK_VIEW,
                    SCREENS::TRACK_EFFECTS_MIDI, SCREENS::TRACK_EFFECTS_AUDIO,
                    SCREENS::MASTER_EFFECTS_MIDI, SCREENS::MASTER_EFFECTS_AUDIO},
                   {MIDI::GENERAL::NOTEON_HEADER, MIDI::GENERAL::NOTEOFF_HEADER,
                    MIDI::GENERAL::CC_HEADER, MIDI::GENERAL::CTRL_HEADER,
                    MIDI::GENERAL::PITCHWHEEL_HEADER, MIDI::GENERAL::NOTE_HEADER},
                   {}, [this](MData &cmd, Sync &sync) -> void {

        if (cmd.status == MIDI::GENERAL::NOTE_HEADER) {
            cmd.status = cmd.data2 > 0 ? MIDI::GENERAL::NOTEON_HEADER : MIDI::GENERAL::NOTEOFF_HEADER;
            cmd.data1 = cmd.data1 + (octave + 5) * 12;
            cmd.data2 = cmd.data2;
        }

        if (cmd.status == MIDI::GENERAL::CC_HEADER) {
            if (cmd.data1 == CC_E1 || cmd.data1 == CC_E3) {
                int d = (cmd.data2 - 64) * enc1_scale;
                if (d < -63) d = -63;
                if (d > 63) d = 63;
                enc1_scale += 100;
                cmd.data2 = 64 + d;
            }
            if (cmd.data1 == CC_E2 || cmd.data1 == CC_E4) {
                int d = (cmd.data2 - 64) * enc2_scale;
                if (d < -63) d = -63;
                if (d > 63) d = 63;
                enc2_scale += 100;
                cmd.data2 = 64 + d;
            }
        }

        if (cmd.status == MIDI::GENERAL::CC_HEADER) {
            switch (SCREEN_IDX) {
                case SCREENS::LOOP_VIEW:
                case SCREENS::TRACK_VIEW:
                    getCurrentTrackInstrument()->midiIn(cmd, sync);
                    break;
                case SCREENS::TAPE_VIEW:
                    break;
                case SCREENS::TRACK_EFFECTS_MIDI:
                    getCurrentTrackMIDIFXRack()->get_focus()->midiIn(cmd, sync);
                    break;
                case SCREENS::TRACK_EFFECTS_AUDIO:
                    getCurrentTrackAudioFXRack()->get_focus()->midiIn(cmd, sync);
                    break;
                case SCREENS::MASTER_EFFECTS_MIDI:
                    master_midi_fx->get_focus()->midiIn(cmd, sync);
                    break;
                case SCREENS::MASTER_EFFECTS_AUDIO:
                    master_audio_fx->get_focus()->midiIn(cmd, sync);
                    break;
                default:
                    break;
            }
        } else {
            master_midi_fx->get_item(0)->midiIn(cmd, sync);
//            tracks->get_focus()->midiIn(cmd, sync);
        }

//        tracks->get_focus()->midiIn(cmd, sync);
        if (SCREEN_IDX != SCREENS::TRACK_VIEW) tapes->midiIn(cmd, sync);
    });

    addMIDIHandler({SCREENS::PROJECT},
                   {MIDI::GENERAL::CC_HEADER},
                   {}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data1 == CC_E1) {
            sync.setBPM(sync.getBPM() + (cmd.data2 - 64) * 0.25);
        }
        if (cmd.data1 == CC_E2 && cmd.data2 > 0) {
            if (line_in) std::system("amixer set 'Capture Mux' 'MIC_IN'");
            else std::system("amixer set 'Capture Mux' 'LINE_IN'");
            line_in = ~line_in;
        }
        cmd.status = MIDI::GENERAL::INVALIDATED;
    });

    addMIDIHandler({},
                   {MIDI::GENERAL::BUTN_HEADER},
                   {MIDI::UI::DAW::UP, MIDI::UI::DAW::DOWN}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 == 0) return;

        Rack * rack = nullptr;
        switch (SCREEN_IDX) {
            case SCREENS::LOOP_VIEW:
            case SCREENS::TAPE_VIEW:
            case SCREENS::TRACK_VIEW:
                rack = tracks;
                break;
            case SCREENS::TRACK_EFFECTS_MIDI:
                rack = getCurrentTrackMIDIFXRack();
                break;
            case SCREENS::TRACK_EFFECTS_AUDIO:
                rack = getCurrentTrackAudioFXRack();
                break;
            case SCREENS::MASTER_EFFECTS_MIDI:
                rack = master_midi_fx;
                break;
            case SCREENS::MASTER_EFFECTS_AUDIO:
                rack = master_audio_fx;
                break;
            default:
                break;
        }

        if (rack == nullptr) return;

        switch (cmd.data1) {
            case MIDI::UI::DAW::UP:
                rack->dive_next();
                break;
            case MIDI::UI::DAW::DOWN:
                rack->dive_prev();
                break;
            default:
                break;
        }

        if ((dynamic_cast<SampleKit*>(((Rack*)((Rack*)tracks->get_focus())->get_item(1))->get_focus()) == nullptr &&
            strcmp(((Rack*)((Rack*)tracks->get_focus())->get_item(1))->get_focus()->getName(), "MICIN") != 0)) {
            master_scale->enable(true);
        } else {
            master_scale->enable(false);
        }
    });

    addMIDIHandler({SCREENS::TAPE_VIEW}, {MIDI::GENERAL::SHFT_HEADER}, {}, [this](MData &cmd, Sync &sync) -> void {
        tapes->midiIn(cmd, sync);
    });

    addMIDIHandler({SCREENS::LOOP_VIEW}, {MIDI::GENERAL::SHFT_HEADER}, {}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 == 0) return;

        switch (cmd.data1) {
            case MIDI::KEYS::C:
            case MIDI::KEYS::Db:
            case MIDI::KEYS::D:
            case MIDI::KEYS::Eb:
            case MIDI::KEYS::E:
            case MIDI::KEYS::F:
            case MIDI::KEYS::Gb:
            case MIDI::KEYS::G:
            case MIDI::KEYS::Ab:
            case MIDI::KEYS::A:
            case MIDI::KEYS::Bb:
            case MIDI::KEYS::B:
            default:
                break;
        }
    });


//    addMIDIHandler({}, {MIDI::GENERAL::SHFT_HEADER}, {MIDI::UI::DAW::DOWNDOWN}, [this](MData &cmd, Sync &sync) -> MIDISTATUS {
//        if (cmd.data2 > 0) {
//            switch (focus_rack_depth) {
//                case 0:
//                    ((Rack *)(((Rack *) tracks->get_focus())->get_item(1)))->dive_prev();
//                    this->focus_rack = (Rack *)(((Rack *) tracks->get_focus())->get_item(1));
//                    break;
//                default:
//                    break;
//            }
//        }
//        return MIDISTATUS::DONE;
//    });
}