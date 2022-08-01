//
// Created by ferluht on 07/07/2022.
//

#include "Daw.h"
#include <cmath>

void DAW::midiIn(MData& cmd) {
    if (cmd.status == CC_HEADER) {
        cc_states[cmd.data1] = cmd.data2;
        if (cc_handlers[cmd.data1]) (this->*(cc_handlers[cmd.data1]))(cmd);
    }

    tracks->midiIn(cmd);
    tapes->midiIn(cmd);
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

void DAW::draw(NVGcontext *vg) {
    focus_rack->draw(vg);
}

void DAW::SHandler(MData &cmd) {

}

void DAW::PHandler(MData &cmd) {

}

void DAW::EHandler(MData &cmd) {
    focus_rack->midiIn(cmd);
}

void DAW::KHandler(MData &cmd) {
    if (cc_states[S1] && cc_states[S2]) {

    } else if (cc_states[S1]) {

    } else if (cc_states[S2] && cc_states[cmd.data1]) {
        switch (cmd.data1) {
            case K1:
                focus_rack = focus_rack->dive_out();
                std::cout << "dive out" << std::endl;
                break;
            case K2:
                focus_rack->dive_next();
                std::cout << "dive next" << std::endl;
                break;
            case K3:
                focus_rack = focus_rack->dive_in();
                std::cout << "dive in" << std::endl;
                break;
            default:
                break;
        }
    } else {
        if (cmd.data2 > 0) cmd.status = NOTEON_HEADER;
        else cmd.status = NOTEOFF_HEADER;
    }
}


Rack *DAW::spawnTracksRack(int n) {
    Rack * tracks_ = new Rack(Rack::SELECTIVE);
//    for (int i = 0; i < n; i ++) {
//        auto tr = spawnSingleTrack();
//        tr->attach(tracks_);
//        tracks_->add(tr);
//    }

    auto tr = spawnSingleTrack(1, 2, 0);
    tr->attach(tracks_);
    tracks_->add(tr);
    tr->dive_next();

    tr = spawnSingleTrack(1, 1, 1);
    tr->attach(tracks_);
    tracks_->add(tr);
    tr->dive_next();

    tr = spawnSingleTrack(1, 3, 1);
    tr->attach(tracks_);
    tracks_->add(tr);
    tr->dive_next();

    return tracks_;
}

Rack *DAW::spawnSingleTrack() {spawnSingleTrack(0,0,0);}

Rack *DAW::spawnSingleTrack(int i, int j, int k) {
    Rack * track = new Rack(Rack::SEQUENTIAL);

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
    Rack * midirack = new Rack(Rack::SELECTIVE);
    midirack->add(new DummyMidi());
    midirack->add(new Scale());
    return midirack;
}

Rack *DAW::spawnInstrumentRack() {
    Rack * instrument = new Rack(Rack::SELECTIVE);
    instrument->add(new DummyInstrument());
    instrument->add(new SingleTone());
    instrument->add(new Sampler("/home/pi/rpidaw_samples/Kick/Kick 909 1.wav"));
    instrument->add(new SimpleInstrument());

    auto kit = new SampleKit();
    kit->addSample("/home/pi/rpidaw_samples/Kick/Kick 909 1.wav", 60);
    kit->addSample("/home/pi/rpidaw_samples/Kick/Kick 70sDnB 1.wav", 61);
    kit->addSample("/home/pi/rpidaw_samples/Snare/Snare 808 1.wav", 62);
    kit->addSample("/home/pi/rpidaw_samples/Snare/Snare 70sDnB 1.wav", 63);
    kit->addSample("/home/pi/rpidaw_samples/HHClosed/ClosedHH 808.wav", 64);
    instrument->add(kit);
    return instrument;
}

Rack *DAW::spawnEffectRack() {
    Rack * effects = new Rack(Rack::SELECTIVE);
    effects->add(new DummyAudio());
    effects->add(new Plateau());
    return effects;
}

Rack *DAW::spawnTapeRack(int n) {
    Rack * tapes = new Rack(Rack::PARALLEL);
    for (int i = 0; i < n; i ++) {
        tapes->add(new Tape());
    }
    return tapes;
}
