//
// Created by ferluht on 07/07/2022.
//

#include "Daw.h"

DAW::DAW (GFXcanvas1 * screen_) : AMG("DAW") {
    screen = screen_;

    initHardwareControls();
    initControlMappings();
    initMidiHandlers();
    initScreens();

    tracks = spawnTracksRack();
//    tapes = new LoopMatrix();
    tapes = new Tape();

    master_midi_fx = new Rack("MASTER MIDI FX", Rack::SEQUENTIAL);
    master_scale = new Scale();
    master_midi_fx->add(master_scale);
    master_midi_fx->add(new Arpeggiator());

    master_audio_fx = tapes->master_effects;
}

void DAW::midiIn(MData& cmd) {
    midiLock.lock();
    midiInQueue.push_back(cmd);
    midiLock.unlock();
}

void DAW::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames) {
    float inbuf[BUF_SIZE * 2];
    float buf[BUF_SIZE * 2];

    for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
        inbuf[i + 0] = inputBuffer[i + 0];
        inbuf[i + 1] = inputBuffer[i + 1];
    }

//    if (strcmp(((Rack*)((Rack*)tracks->get_focus())->get_item(1))->get_focus()->getName(), "MICIN") == 0) {
//        for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
//            inbuf[i + 0] = inputBuffer[i + 0];
//            inbuf[i + 1] = inputBuffer[i + 1];
//        }
//    } else {
//        float j = -1;
//        for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
//            inbuf[i + 0] = 0.0001f * j;
//            inbuf[i + 1] = 0.0001f * j;
//            j *= -1;
//        }
//    }

    float j = -1;
    for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
        buf[i + 0] = 0.0001f * j;
        buf[i + 1] = 0.0001f * j;
        j *= -1;
    }

    while (!midiInQueue.empty()) {
        MData cmd = midiInQueue.front();
        midiMap->midiIn(cmd, sync);
        M::midiIn(cmd, sync);
        midiLock.lock();
        midiInQueue.pop_front();
        midiLock.unlock();
    }

    midiOut(midiOutQueue, sync);

//    if (tapes->getState() != Tape::TAPE_STATE::REWIND)
    tracks->process(buf, inbuf, nBufferFrames, sync);
    j = -1;
    for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
        buf[i + 0] += 0.0001f * j;
        buf[i + 1] += 0.0001f * j;
        j *= -1;
    }
    tapes->process(inbuf, buf, nBufferFrames, sync);

    for (unsigned int i=0; i<2*nBufferFrames; i++ ) outputBuffer[i] = inbuf[i] * 0.5; // soft_clip(inbuf[i] * 0.5);

    if (recording_master) sr.writeSamples(outputBuffer, nBufferFrames);

    if (enc1_scale > 1) enc1_scale -= 1;
    if (enc2_scale > 1) enc2_scale -= 1;
}

Rack *DAW::spawnTracksRack() {
    Rack * tracks_ = new Rack("ARRANGEMENT",Rack::SELECTIVE);

    for (int i = 0; i < 9; i ++) {
        auto tr = spawnSingleTrack("TRACK", 0, i, 0);
        tr->attach(tracks_);
        tracks_->add(tr);
        tr->dive_next();
    }

    return tracks_;
}

Rack *DAW::spawnSingleTrack() {return spawnSingleTrack("TRACK",0,0,0);}

Rack *DAW::spawnSingleTrack(const char * name, int i, int j, int k) {
    Rack * track = new Rack(name, Rack::SEQUENTIAL);

    auto r = spawnMidiFXRack();
    r->attach(track);
    track->add(r);
    r->set_focus_by_index(i);

    r = spawnInstrumentRack();
    r->attach(track);
    track->add(r);
    r->set_focus_by_index(j);

    r = spawnAudioFXRack();
    r->attach(track);
    track->add(r);
    r->set_focus_by_index(k);

    return track;
}

Rack *DAW::spawnMidiFXRack() {
    Rack * midirack = new Rack("MIDI FX",Rack::SEQUENTIAL);
    midirack->add(new Arpeggiator());
//    midirack->add(new DummyMidiFX());
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

Rack *DAW::spawnAudioFXRack() {
    Rack * effects = new Rack("AUDIO FX", Rack::SELECTIVE);
    effects->add(new Delay());
    effects->add(new DummyAudioFX());
    return effects;
}

void DAW::midiOut(std::deque<MData> &q, Sync & sync) {
    master_midi_fx->midiOut(midiOutQueue, sync);
    tracks->midiOut(midiOutQueue, sync);
    tapes->midiOut(midiOutQueue, sync);
    midiOutQueue.clear();
}
