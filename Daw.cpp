//
// Created by ferluht on 07/07/2022.
//

#include "Daw.h"
#include <Adafruit-GFX-offscreen/Fonts/Picopixel.h>
#include <cmath>

MIDISTATUS DAW::midiIn(MData& cmd) {
    if (dawMidiStatus != MIDISTATUS::DONE)
        return MIDISTATUS::DONE;

    midiLock.lock();
    midiQueue.push_back(cmd);
    midiLock.unlock();

    return MIDISTATUS::DONE;
}

void DAW::process(float *outputBuffer, float *inputBuffer,
             unsigned int nBufferFrames, double streamTime) {
    float buf[BUF_SIZE * 2];
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

        bool tracks_done = tracks->midiIn(cmd) == MIDISTATUS::DONE;
        bool tapes_done = tapes->midiIn(cmd) == MIDISTATUS::DONE;

        if (tracks_done && tapes_done) {
            midiLock.lock();
            midiQueue.pop_front();
            midiLock.unlock();
        } else {
            ms = MIDISTATUS::WAITING;
        }
    }

    dawMidiStatus = ms;

    tracks->process(buf, buf, nBufferFrames, 0);
    tapes->process(buf, buf, nBufferFrames, 0);

    for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
        buf[i + 0] *= 0.5;
        buf[i + 1] *= 0.5;
    }

    for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
        *outputBuffer++ = buf[i + 0];
        *outputBuffer++ = buf[i + 1];
        envelope(buf[i + 0], 0.01, 0.01);
    }
}

void DAW::draw(GFXcanvas1 * screen) {
    screen->drawFastVLine(96, 0, 32, 1);
    screen->drawFastHLine(0, 8, 128, 1);

    screen->setFont(&Picopixel);
    screen->setTextSize(1);

    auto selected_track = (Rack *) (tracks->get_focus());
    screen->setCursor(4, 6);
    screen->setTextSize(1);
    screen->print(selected_track->name);

    auto selected_stage = (Rack *) (selected_track->get_focus());
    screen->setCursor(34, 6);
    screen->setTextSize(1);
    screen->print(selected_stage->name);

    auto selected_fx = (Rack *) (selected_stage->get_focus());
    screen->setCursor(66, 6);
    screen->setTextSize(1);
    screen->print(selected_fx->name);

    if (focus_rack == tracks) {
        screen->drawFastHLine(0, 9, 32, 1);
    } else if (focus_rack == selected_track) {
        screen->drawFastHLine(32, 9, 32, 1);
    } else {
        screen->drawFastHLine(64, 9, 32, 1);
    }

    screen->setCursor(101, 6);
    screen->setTextSize(1);
    screen->print(tapes->name);

    focus_rack->draw(screen);
    tapes->draw(screen);

    if (dawMidiStatus == MIDISTATUS::WAITING) {
        screen->fillRect(16, 4, 96, 24, 0);
        screen->drawRect(16, 4, 96, 24, 1);
        screen->setCursor(32, 20);
        screen->setTextSize(2);
        screen->print("WAITING...");
    }
}

//void DAW::KHandler(MData &cmd) {
//    if (cc_states[S1] && cc_states[S2]) {
//
//    } else if (cc_states[S1]) {
//
//    } else if (cc_states[S2] && cc_states[cmd.data1]) {
//        switch (cmd.data1) {
//            case K1:
//                focus_rack = focus_rack->dive_out();
//                break;
//            case K2:
//                focus_rack->dive_next();
//                break;
//            case K3:
//                focus_rack = focus_rack->dive_in();
//                break;
//            case K4:
//                focus_rack->dive_prev();
//                break;
//            default:
//                break;
//        }
//    } else {
//        if (cmd.data2 > 0) cmd.status = NOTEON_HEADER;
//        else cmd.status = NOTEOFF_HEADER;
//    }
//}


Rack *DAW::spawnTracksRack(int n) {
    Rack * tracks_ = new Rack("ARRANGEMENT",Rack::SELECTIVE);

    auto tr = spawnSingleTrack("TRACK 0", 1, 3, 1);
    tr->attach(tracks_);
    tracks_->add(tr);
    tr->dive_next();

    tr = spawnSingleTrack("TRACK 1",1, 1, 1);
    tr->attach(tracks_);
    tracks_->add(tr);
    tr->dive_next();

    tr = spawnSingleTrack("TRACK 2",1, 4, 0);
    tr->attach(tracks_);
    tracks_->add(tr);
    tr->dive_next();

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
    midirack->add(new DummyMidi());
    midirack->add(new Scale());
    return midirack;
}

Rack *DAW::spawnInstrumentRack() {
    Rack * instrument = new Rack("ENGINE", Rack::SELECTIVE);
    instrument->add(new DummyInstrument());
    instrument->add(new SingleTone());
    instrument->add(new Sampler("../res/samples/Kick/Kick 909 1.wav"));
    instrument->add(new SimpleInstrument());

    auto kit = new SampleKit();
    kit->addSample("../res/samples/Kick/Kick 909 1.wav", 60);
    kit->addSample("../res/samples/Kick/Kick 70sDnB 1.wav", 61);
    kit->addSample("../res/samples/Snare/Snare 808 1.wav", 62);
    kit->addSample("../res/samples/Snare/Snare 70sDnB 1.wav", 63);
    kit->addSample("../res/samples/HHClosed/ClosedHH 808.wav", 64);
    instrument->add(kit);
    return instrument;
}

Rack *DAW::spawnEffectRack() {
    Rack * effects = new Rack("AUDIO FX", Rack::SELECTIVE);
    effects->add(new DummyAudio());
    effects->add(new Plateau());
    return effects;
}

Rack *DAW::spawnTapeRack(int n) {
    Rack * tapes = new Rack("TAPES",Rack::PARALLEL);
    for (int i = 0; i < n; i ++) {
        tapes->add(new Tape());
    }
    return tapes;
}
