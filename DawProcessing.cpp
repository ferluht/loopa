//
// Created by ferluht on 07/07/2022.
//

#include "Daw.h"
#include <Adafruit-GFX-offscreen/Fonts/Picopixel.h>
#include <cmath>

DAW::DAW (GFXcanvas1 * screen_) : AMG("DAW") {
    screen = screen_;

    initMidiHandlers();

    tracks = spawnTracksRack();
    tapes = new LoopMatrix();

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

MIDISTATUS DAW::midiIn(MData& cmd) {
    if (dawMidiStatus != MIDISTATUS::DONE)
        return MIDISTATUS::DONE;

//    std::cout << (int) cmd.status << " " << (int) cmd.data1 << "\n";

    midiLock.lock();
    midiQueue.push_back(cmd);
    midiLock.unlock();

    return MIDISTATUS::DONE;
}

void DAW::process(float *outputBuffer, float *inputBuffer,
             unsigned int nBufferFrames, double streamTime) {
    float inbuf[BUF_SIZE * 2];
    float buf[BUF_SIZE * 2];

    if (strcmp(((Rack*)((Rack*)tracks->get_focus())->get_item(1))->get_focus()->getName(), "MICIN") == 0) {
        for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
            inbuf[i + 0] = inputBuffer[i + 0];
            inbuf[i + 1] = inputBuffer[i + 1];
        }
    } else {
        float j = -1;
        for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
            inbuf[i + 0] = 0.0001f * j;
            inbuf[i + 1] = 0.0001f * j;
            j *= -1;
        }
    }

    float j = -1;
    for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
        buf[i + 0] = 0.0001f * j;
        buf[i + 1] = 0.0001f * j;
        j *= -1;
    }

    MIDISTATUS ms = MIDISTATUS::DONE;
    while (ms == MIDISTATUS::DONE && !midiQueue.empty()) {

        MData cmd = midiQueue.front();
        midiMap->midiIn(cmd);
        M::midiIn(cmd);

        if (current_screen == DAW::FSCREEN::MAIN) {
            bool tracks_done = false;
            if (cmd.status == MIDI::GENERAL::CC_HEADER) {
                tracks_done = focus_rack->midiIn(cmd) == MIDISTATUS::DONE;
            } else {
                if (dynamic_cast<SampleKit*>(((Rack*)((Rack*)tracks->get_focus())->get_item(1))->get_focus()) == nullptr &&
                    strcmp(((Rack*)((Rack*)tracks->get_focus())->get_item(1))->get_focus()->getName(), "MICIN") != 0) {
                    master_fx->get_item(0)->midiIn(cmd);
                }
                tracks_done = tracks->midiIn(cmd) == MIDISTATUS::DONE;
            }

            bool tapes_done = tapes->midiIn(cmd) == MIDISTATUS::DONE;

            if (tracks_done && tapes_done) {
                midiLock.lock();
                midiQueue.pop_front();
                midiLock.unlock();
            } else {
                ms = MIDISTATUS::WAITING;
            }
        } else if (current_screen == DAW::FSCREEN::LOOP) {
            bool tracks_done = true;
            if (cmd.status != MIDI::GENERAL::CC_HEADER) {
                if (dynamic_cast<SampleKit*>(((Rack*)((Rack*)tracks->get_focus())->get_item(1))->get_focus()) == nullptr) {
                    master_fx->get_item(0)->midiIn(cmd);
                }
                tracks_done = tracks->midiIn(cmd) == MIDISTATUS::DONE;
            }

            bool fx_done = master_fx->get_focus()->midiIn(cmd) == MIDISTATUS::DONE;

            if (fx_done && tracks_done) {
                midiLock.lock();
                midiQueue.pop_front();
                midiLock.unlock();
            } else {
                ms = MIDISTATUS::WAITING;
            }
        }
    }

    dawMidiStatus = ms;

    tracks->process(buf, inbuf, nBufferFrames, 0);
    tapes->process(buf, buf, nBufferFrames, 0);

    for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
        buf[i + 0] *= 0.5;
        buf[i + 1] *= 0.5;
    }

    for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
        outputBuffer[i + 0] = soft_clip(buf[i + 0]);
        outputBuffer[i + 1] = soft_clip(buf[i + 1]);
        envelope(buf[i + 0], 0.01, 0.01);
    }

    if (recording_master) {
        sr.writeSamples(outputBuffer, nBufferFrames);
    }
}

void DAW::drawMainScreen(GFXcanvas1 *screen) {
    screen->drawFastVLine(60, 2, 5, 1);
    screen->drawFastVLine(78, 2, 5, 1);
    screen->drawFastVLine(96, 2, 5, 1);
    screen->drawFastHLine(0, 8, 128, 1);

    screen->setFont(&Picopixel);
    screen->setTextSize(1);

    int16_t s = 81;

    for (int i = 0; i < 14; i ++)
        screen->drawPixel(s+i, 4 + 2 * sin(((float)i / 14) * 2 * M_PI), 1);

    s = 64;
    screen->drawFastHLine(s-1, 2, 5, 1);
    screen->drawFastHLine(s+3, 6, 4, 1);
    screen->drawFastVLine(s+3, 3, 3, 1);
    screen->drawFastVLine(s+6, 3, 3, 1);
    screen->drawFastHLine(s+6, 2, 3, 1);
    screen->drawFastVLine(s+8, 3, 3, 1);
    screen->drawFastHLine(s+8, 6, 3, 1);

    auto selected_track = (Rack *) (tracks->get_focus());
    screen->setCursor(4, 6);
    screen->setTextSize(1);
    screen->print(std::to_string(tracks->get_focus_index()).c_str());

    screen->drawFastVLine(12, 2, 2, 1);
    screen->drawFastVLine(12, 5, 2, 1);

    auto selected_instrument_rack = (Rack *) (selected_track->get_item(1));
    screen->setCursor(16, 6);
    screen->setTextSize(1);
    auto instr_name = selected_instrument_rack->get_focus()->getName();
    screen->print(instr_name);

    focus_rack->draw(screen);

    if (focus_rack == selected_track->get_item(1)) {
        screen->drawFastHLine(0, 9, 60, 1);
    } else if (focus_rack == selected_track->get_item(0)) {
        screen->drawFastHLine(60, 9, 18, 1);
    } else {
        screen->drawFastHLine(78, 9, 18, 1);
    }

    screen->setCursor(103, 6);
    screen->setTextSize(1);
    screen->print("TAPES");

    screen->drawFastVLine(96, 0, 32, 1);

    tapes->draw(screen);

    if (dawMidiStatus == MIDISTATUS::WAITING) {
        screen->fillRect(16, 4, 96, 24, 0);
        screen->drawRect(16, 4, 96, 24, 1);
        screen->setCursor(32, 20);
        screen->setTextSize(2);
        screen->print("WAITING...");
    }
}

void DAW::drawLoopScreen(GFXcanvas1 *screen) {
    screen->drawFastVLine(18, 2, 5, 1);
    screen->drawFastVLine(36, 2, 5, 1);
    screen->drawFastHLine(0, 8, 128, 1);

    int16_t s = 21;

    for (int i = 0; i < 14; i ++)
        screen->drawPixel(s+i, 4 + 2 * sin(((float)i / 14) * 2 * M_PI), 1);

    s = 4;
    screen->drawFastHLine(s-1, 2, 5, 1);
    screen->drawFastHLine(s+3, 6, 4, 1);
    screen->drawFastVLine(s+3, 3, 3, 1);
    screen->drawFastVLine(s+6, 3, 3, 1);
    screen->drawFastHLine(s+6, 2, 3, 1);
    screen->drawFastVLine(s+8, 3, 3, 1);
    screen->drawFastHLine(s+8, 6, 3, 1);

    if (master_fx->get_focus() == master_fx->get_item(0)) {
        screen->drawFastHLine(0, 9, 18, 1);
    } else if (master_fx->get_focus() == master_fx->get_item(1)) {
        screen->setCursor(103, 6);
        screen->setTextSize(1);
        screen->print("TAPES");

        screen->drawFastVLine(96, 0, 32, 1);

        screen->drawFastHLine(18, 9, 18, 1);
    }

    master_fx->get_focus()->draw(screen);
}

void DAW::draw(GFXcanvas1 * screen) {

    screen->leds[3] = recording_master * 50;

    switch (current_screen) {
        case (FSCREEN::LOOP):
            drawLoopScreen(screen);
            break;
        case (FSCREEN::MAIN):
            drawMainScreen(screen);
            break;
        default:
            break;
    }
}

Rack *DAW::spawnTracksRack() {
    Rack * tracks_ = new Rack("ARRANGEMENT",Rack::SELECTIVE);

    for (int i = 0; i < 8; i ++) {
        auto tr = spawnSingleTrack("TRACK", 0, i, 0);
        tr->attach(tracks_);
        tracks_->add(tr);
        tr->dive_next();
    }

    return tracks_;
}

Rack *DAW::spawnSingleTrack() {spawnSingleTrack("TRACK",0,0,0);}

Rack *DAW::spawnSingleTrack(const char * name, int i, int j, int k) {
    Rack * track = new Rack(name, Rack::SEQUENTIAL);

    auto r = spawnMidiRack();
    r->attach(track);
    track->add(r);
    r->set_focus_by_index(i);

    r = spawnInstrumentRack();
    r->attach(track);
    track->add(r);
    r->set_focus_by_index(j);

    r = spawnEffectRack();
    r->attach(track);
    track->add(r);
    r->set_focus_by_index(k);

    return track;
}

Rack *DAW::spawnMidiRack() {
    Rack * midirack = new Rack("MIDI FX",Rack::SELECTIVE);
    midirack->add(new DummyMidiFX());
    return midirack;
}

Rack *DAW::spawnInstrumentRack() {
    Rack * instrument = new Rack("ENGINE", Rack::SELECTIVE);
    instrument->add(new SimpleInstrument());
    instrument->add(new MicInput());
    instrument->add(new SingleTone());

    std::string base_path = "../res/samples";
    auto dirs = list_dir(base_path.c_str());
    for (auto & dir : dirs) {
        try {
            std::string samplepack_path = base_path.append("/") + dir;
            auto files = list_dir(samplepack_path.c_str());
            int nfiles = 0;
            for (auto &f: files)
                if (f.find(".wav") != std::string::npos) nfiles++;
            if (nfiles > 1) {
                auto kit = new SampleKit(dir.c_str());
                bool valid = false;
                for (auto &f: files) {
                    if (f.find(".wav") == std::string::npos) continue;
                    std::string notenum = f.substr(f.find("_") + 1, std::string::npos);
                    notenum = notenum.substr(0, notenum.find("."));
                    int8_t note = std::stoi(notenum);
                    std::string sample_path = samplepack_path.append("/") + f;
                    kit->addSample(sample_path.c_str(), note);
                    valid = true;
                }
                if (valid) instrument->add(kit);
            } else if (nfiles == 1) {
                for (auto &f: files) {
                    if (f.find(".wav") == std::string::npos) continue;
                    std::string notenum = f.substr(f.find("_") + 1, std::string::npos);
                    notenum = notenum.substr(0, notenum.find("."));
                    int8_t note = std::stoi(notenum);
                    std::string sample_path = samplepack_path.append("/") + f;
                    auto sampler = new Sampler(dir.c_str(), sample_path.c_str(), note);
                    instrument->add(sampler);
                    break;
                }
            }
        } catch (...) {
            std::cout << "unable to open pack " << dir << std::endl;
        }
    }

    return instrument;
}

Rack *DAW::spawnEffectRack() {
    Rack * effects = new Rack("AUDIO FX", Rack::SELECTIVE);
    effects->add(new DummyAudioFX());
//    effects->add(new Delay());
    return effects;
}

Rack *DAW::spawnTapeRack(int n) {
    Rack * tapes = new Rack("TAPES",Rack::PARALLEL);
    for (int i = 0; i < n; i ++) {
        tapes->add(new Tape());
    }
    return tapes;
}
