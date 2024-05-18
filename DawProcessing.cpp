//
// Created by ferluht on 07/07/2022.
//

#include "Daw.h"

OnboardingNode::OnboardingNode(std::function<bool(MData &, Sync &, int& state)> condition,
                               std::function<void(GFXcanvas1 *, int& state)> drawer,
                               bool block, int time_delay) {
    this->condition = condition;
    this->drawer = drawer;
    this->block = block;
    this->time_delay = time_delay;
    this->draw_calls_counter = 0;
    this->state = 0;
}

void drawBanner(GFXcanvas1 * screen, std::vector<std::string> lines) {
    screen->fillRect(14, 2, 100, 28, 0);
    screen->drawRect(16, 4, 96, 24, 1);
    screen->setTextSize(1);

    int16_t x1, y1;
    uint16_t w, h;
    for (int i = 0; i < lines.size(); i ++) {
        screen->getTextBounds(lines[i], 0, 0, &x1, &y1, &w, &h);
        screen->setCursor((96 - w) / 2 + 16, (24 - (h + 4) * lines.size()) / 2 + 4 + h / 2 + 4 + (h+4) * i);
        screen->print(lines[i].c_str());
    }
}

OnboardingNode OnboardingNode::createBannerQuestion(std::vector<std::string> lines, int time_delay) {
    auto q = OnboardingNode(
            [](MData &cmd, Sync &sync, int& state){
                if (cmd.status == MIDI::GENERAL::CC_HEADER && cmd.data1 == 101) {
                    state = -1;
                    return true;
                }
                if ((cmd.status == MIDI::GENERAL::CC_HEADER && cmd.data1 == 102) ||
                    (cmd.status == MIDI::GENERAL::CC_HEADER && cmd.data1 == 100)){
                    return true;
                }
                return false;
            },
            [lines](GFXcanvas1 * screen, int& state){
                drawBanner(screen, lines);
                screen->setLed(0, 10, 0, 0);
                screen->setLed(1, 0, 10, 0);
            },
            true,
            time_delay
    );
    q.flash_period = 1;
    q.on_percentage = 1;
    return q;
}

OnboardingNode OnboardingNode::createBannerExitTime(std::vector<std::string> lines, int time_delay, int time_exit) {
    return OnboardingNode(
            [time_exit](MData &cmd, Sync &sync, int& state){
                cmd.status = MIDI::GENERAL::INVALIDATED;
                if (state >= time_exit) return true;
                return false;
            },
            [lines](GFXcanvas1 * screen, int& state){
                drawBanner(screen, lines);
                state++;
            },
            false,
            time_delay
    );
}

OnboardingNode OnboardingNode::createBannerExitAny(std::vector<std::string> lines, int time_delay) {
    return OnboardingNode(
            [](MData &cmd, Sync &sync, int& state){
                cmd.status = MIDI::GENERAL::INVALIDATED;
                return true;
            },
            [lines](GFXcanvas1 * screen, int& state){
                drawBanner(screen, lines);
            },
            false,
            time_delay
    );
}

OnboardingNode OnboardingNode::createEmptyExitStatus(uint8_t status,
                                                     bool block, int time_delay) {
    return OnboardingNode(
            [status](MData &cmd, Sync &sync, int& state){
                if (cmd.status == status) return true;
                return false;
            },
            [](GFXcanvas1 * screen, int& state){},
            block,
            time_delay
    );
}

OnboardingNode OnboardingNode::createEmptyExitStatusAndData(uint8_t status,
                                                            uint8_t data1,
                                                            bool block, int time_delay){
    return OnboardingNode(
            [status, data1](MData &cmd, Sync &sync, int& state){
                if (cmd.status == status && cmd.data1 == data1) return true;
                return false;
            },
            [](GFXcanvas1 * screen, int& state){},
            block,
            time_delay
    );
}

OnboardingNode OnboardingNode::createBannerExitStatus(std::vector<std::string> lines,
                                                      uint8_t status,
                                                      bool block, int time_delay) {
    return OnboardingNode(
            [status](MData &cmd, Sync &sync, int& state){
                if (cmd.status == status) return true;
                return false;
            },
            [lines](GFXcanvas1 * screen, int& state){
                drawBanner(screen, lines);
            },
            block,
            time_delay
    );
}

OnboardingNode OnboardingNode::createBannerExitStatusAndData(std::vector<std::string> lines,
                                                             uint8_t status,
                                                             uint8_t data1,
                                                             bool block, int time_delay){
    return OnboardingNode(
            [status, data1](MData &cmd, Sync &sync, int& state){
                if (cmd.status == status && cmd.data1 == data1) return true;
                return false;
            },
            [lines](GFXcanvas1 * screen, int& state){
                drawBanner(screen, lines);
            },
            block,
            time_delay
    );
}

OnboardingNode OnboardingNode::createBannerExitStatusStartStop(std::vector<std::string> lines,
                                                               uint8_t status_start,
                                                               uint8_t status_stop,
                                                               bool block, int time_delay) {
    return OnboardingNode(
            [status_start, status_stop](MData &cmd, Sync &sync, int& state){
                if (cmd.status == status_start) state = 1;
                if (state == 1 && cmd.status == status_stop) return true;
                return false;
            },
            [lines](GFXcanvas1 * screen, int& state){
                if (state == 0) drawBanner(screen, lines);
            },
            block,
            time_delay
    );
}

Onboarding::Onboarding() : AMG("Onboarding") {
    reset();
}

void Onboarding::reset() {
    script = {
            OnboardingNode::createBannerExitTime({"WELCOME TO THE LOOPA!"}),
            OnboardingNode::createBannerQuestion({"HEY, MAYBE TUTORIAL?", "NO!                             YES!"}),
            OnboardingNode::createBannerExitStatusAndData({"OKAY, PRESS PLAY TO STOP"}, MIDI::GENERAL::CTRL_HEADER, MIDI::UI::PLAY_BUTTON),
            OnboardingNode::createBannerExitTime({"LET'S CLEAR ALL TRACKS"}),
            OnboardingNode::createBannerExitStatusAndData({"HOLD ALT & PRESS 1", "TO CLEAR 1ST TRACK"}, MIDI::GENERAL::LOOP_HEADER+0, MIDI::UI::TAPE::CLEAR),
            OnboardingNode::createBannerExitStatusAndData({"HOLD ALT & PRESS 2", "TO CLEAR 2ND TRACK"}, MIDI::GENERAL::LOOP_HEADER+1, MIDI::UI::TAPE::CLEAR, true, 2),
            OnboardingNode::createBannerExitStatusAndData({"HOLD ALT & PRESS 3", "TO CLEAR 3RD TRACK"}, MIDI::GENERAL::LOOP_HEADER+2, MIDI::UI::TAPE::CLEAR, true, 2),
            OnboardingNode::createBannerExitStatusAndData({"HOLD ALT & PRESS 4", "TO CLEAR 4TH TRACK"}, MIDI::GENERAL::LOOP_HEADER+3, MIDI::UI::TAPE::CLEAR, true, 2),
            OnboardingNode::createBannerExitStatusAndData({"PRESS 1 TO SWITCH", "TO FIRST TRACK"}, MIDI::GENERAL::LOOP_HEADER+0, MIDI::UI::TAPE::TRIG),
            OnboardingNode::createBannerExitStatusAndData({"HOLD SHIFT & PUSH PLAY", "TO RECORD SOMETHING"}, MIDI::GENERAL::CTRL_HEADER, MIDI::UI::REC_BUTTON),
            OnboardingNode::createBannerExitStatusAndData({"HOLD ALT & PUSH PLAY", "TO TURN ON CLICK"}, MIDI::GENERAL::CTRL_HEADER, MIDI::UI::CLICK_BUTTON),
            OnboardingNode::createBannerExitStatus({"USE ARROWS", "TO SWITCH OCTAVES"}, MIDI::GENERAL::BUTN_HEADER, true, 30),
            OnboardingNode::createBannerExitStatusAndData({"PUSH PLAY WHEN READY", "TO FINISH RECORDING"}, MIDI::GENERAL::CTRL_HEADER, MIDI::UI::PLAY_BUTTON, false),
            OnboardingNode::createBannerExitTime({"LET'S SWITCH INSTRUMENT"}),
            OnboardingNode::createBannerExitStatus({"HOLD PAGE & USE ARROWS", "TO SCROLL INSTRUMENTS"}, MIDI::GENERAL::BUTN_HEADER),
            OnboardingNode::createEmptyExitStatus(MIDI::GENERAL::CC_HEADER),
            OnboardingNode::createBannerExitStatusAndData({"PRESS 2 TO SWITCH", "TO SECOND TRACK"}, MIDI::GENERAL::LOOP_HEADER+1, MIDI::UI::TAPE::TRIG),
            OnboardingNode::createBannerExitTime({"LET'S RECORD SOMETHING", "TO SECOND TRACK"}),
            OnboardingNode::createBannerExitStatusAndData({"HOLD SHIFT & PUSH PLAY", "TO ENABLE RECORDING"}, MIDI::GENERAL::CTRL_HEADER, MIDI::UI::REC_BUTTON),
            OnboardingNode::createBannerExitStatusAndData({"PRESS PLAY WHEN READY", "TO FINISH RECORDING"}, MIDI::GENERAL::CTRL_HEADER, MIDI::UI::PLAY_BUTTON, false),
            OnboardingNode::createBannerExitStatusAndData({"HOLD ALT THEN PUSH PLAY", "TO TURN OFF CLICK"}, MIDI::GENERAL::CTRL_HEADER, MIDI::UI::CLICK_BUTTON),
            OnboardingNode::createBannerExitTime({"LET'S SEE OTHER SCREENS"}),
            OnboardingNode::createBannerExitStatusAndData({"HOLD SHIFT THEN PUSH F#", "NOTE ON KEYBOARD"}, MIDI::GENERAL::PAGE_HEADER, SCREENS::MASTER_EFFECTS_AUDIO),
            OnboardingNode::createBannerExitTime({"HERE ARE MASTER SENDS", "REVERB & DELAY FOR NOW"}, 20, 30),
            OnboardingNode::createBannerExitStatusAndData({"HOLD SHIFT THEN PUSH F", "NOTE ON KEYBOARD"}, MIDI::GENERAL::PAGE_HEADER, SCREENS::MASTER_EFFECTS_MIDI),
            OnboardingNode::createBannerExitTime({"HERE ARE MASTER MIDI FX", "EG KEYBOARD SCALE"}, 20, 30),
            OnboardingNode::createBannerExitTime({"NOW LET'S GO BACK", "TO THE TAPE SCREEN"}, 10, 30),
            OnboardingNode::createBannerExitStatusAndData({"HOLD SHIFT THEN PUSH C#", "NOTE ON KEYBOARD"}, MIDI::GENERAL::PAGE_HEADER, SCREENS::TAPE_VIEW),
            OnboardingNode::createBannerExitTime({"THAT'S IT", "SEE MORE IN CHEATSHEET"}, 10, 30),
    };
    current_pos = script.begin();
}

void Onboarding::midiIn(MData &cmd, Sync &sync) {
//    draw_counter = 0;
    if (current_pos != script.end()) {
        if (current_pos->condition(cmd, sync, current_pos->state)) {
            if (cmd.data2 > 0) return;
            if (current_pos->state >= 0) current_pos ++;
            else current_pos = script.end();
        }
        else if (cmd.status != MIDI::GENERAL::NOTE_HEADER &&
                 cmd.status != MIDI::GENERAL::NOTEON_HEADER &&
                 cmd.status != MIDI::GENERAL::NOTEOFF_HEADER &&
                 current_pos->block)
            cmd.status = MIDI::GENERAL::INVALIDATED;
    }
}

void Onboarding::draw(GFXcanvas1 *screen) {
    if (current_pos != script.end() &&
        current_pos->draw_calls_counter >= current_pos->time_delay &&
        draw_counter < (current_pos->flash_period * current_pos->on_percentage)) {
        current_pos->drawer(screen, current_pos->state);
    }
    MData cmd = {0,0,0,0};
    Sync sync;
    if (current_pos != script.end() &&
        current_pos->condition(cmd, sync, current_pos->state))
        current_pos++;
    if (current_pos != script.end() &&
        current_pos->draw_calls_counter >= current_pos->time_delay)
        draw_counter = (draw_counter + 1) % current_pos->flash_period;
    if (current_pos != script.end() &&
        current_pos->draw_calls_counter < current_pos->time_delay)
        current_pos->draw_calls_counter ++;
}

Keyboard::Keyboard() : AMG("Keyboard") {

    addMIDIHandler({}, {MIDI::GENERAL::BUTN_HEADER}, {98, 99}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 == 0) return;
        if (cmd.data1 == 98) start_letter -= 12;
        else start_letter += 12;
        if (start_letter < 0) start_letter = 0;
        if (start_letter > 81) start_letter = 81;
    });

    addMIDIHandler({}, {MIDI::GENERAL::NOTE_HEADER}, {}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 == 0) return;
        entry += cmd.data1 + start_letter + '!';
    });

    addMIDIHandler({}, {MIDI::GENERAL::CC_HEADER}, {101, 102}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 == 0) return;
        if (cmd.data1 == 102) onclose(entry);
        isactive = false;
    });

    addMIDIHandler({}, {MIDI::GENERAL::CTRL_HEADER}, {MIDI::UI::PLAY_BUTTON}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 == 0) return;
        if (!entry.empty()) entry = entry.substr(0, entry.size() - 1);
    });

    addDrawHandler({}, [this](GFXcanvas1 * screen) -> void {
        screen->fillRect(14, 2, 100, 28, 0);
        screen->drawRect(16, 4, 96, 24, 1);

        int16_t x1, y1;
        uint16_t w, h;
        screen->getTextBounds(entry.c_str(), 0, 0, &x1, &y1, &w, &h);
        screen->setCursor(110 - w, 10);
        screen->print(entry.c_str());

        for (int i = 0; i < 18; i ++) screen->leds[i] = 0;
        screen->setLed(0, 5, 0, 0);
        screen->setLed(1, 0, 5, 0);
        screen->drawFastHLine(16, 12, 96, 1);
        screen->setCursor(18, 10);
        screen->setTextSize(1);
        screen->print(title.c_str());

        int vo[] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
        for (int i = 0; i < 12; i ++) {
            screen->setCursor(19 + i * 8, 24 - vo[i] * 5);
            char c = start_letter + i + '!';
            screen->print(&c);
        }
    });

}

void Keyboard::enable(std::string title_, std::string initial_entry_, std::function<void(std::string)> onclose_) {
    isactive = true;
    title = std::move(title_);
    entry = std::move(initial_entry_);
    onclose = std::move(onclose_);
}



DAW::DAW (GFXcanvas1 * screen_) : AMG("DAW") {
    read_wlan_settings();

    screen = screen_;

    auto midi_list = DeviceFactory::getDeviceList(DEVICE_TYPE::MIDI_FX);
    auto audio_list = DeviceFactory::getDeviceList(DEVICE_TYPE::AUDIO_FX);
    auto instr_list = DeviceFactory::getDeviceList(DEVICE_TYPE::INSTRUMENT);

    initHardwareControls();
    initControlMappings();
    initMidiHandlers();
    initScreens();

    tracks = spawnTracksRack();
//    tapes = new LoopMatrix();
    tapes = new Tape();

    master_midi_fx = new Rack(Rack::SEQUENTIAL);
    master_scale = new Scale();
    master_midi_fx->add(master_scale);
    master_midi_fx->add(new Arpeggiator());

    master_audio_fx = tapes->master_effects;

    dawState = NORMAL;
}

void DAW::midiIn(MData& cmd) {
    midiLock.lock();
    midiInQueue.push_back(cmd);
    midiLock.unlock();
}

void DAW::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames) {

    dawStateMutex.lock();

    if (dawState != NORMAL) {
        for (unsigned int i=0; i<2*nBufferFrames; i++ ) outputBuffer[i] = 0;
        dawStateMutex.unlock();
        return;
    }

    float inbuf[AUDIO_BUF_SIZE * 2];
    float buf[AUDIO_BUF_SIZE * 2];

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

    last_midi_time ++;
    while (!midiInQueue.empty()) {
        reload_mutex.lock();
        reloaded = false;
        reload_mutex.unlock();
        last_midi_time = 0;
        MData cmd = midiInQueue.front();
        midiMap->midiIn(cmd, sync);
        obd.midiIn(cmd, sync);
        if (kbd.isactive) {
            kbd.midiIn(cmd, sync);
        } else {
            M::midiIn(cmd, sync);
        }
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

    dawStateMutex.unlock();
}

Rack *DAW::spawnTracksRack() {
    Rack * tracks_ = new Rack(Rack::SELECTIVE);

    auto midi_effects = DeviceFactory::getDeviceList(MIDI_FX);
    auto instruments = DeviceFactory::getDeviceList(INSTRUMENT);
    auto audio_effects = DeviceFactory::getDeviceList(AUDIO_FX);

    for (const auto & i: instruments) {
        if (i != "Sampler" && i != "SampleKit" && i != "MicInput") {
            Rack * track = new Rack(Rack::SEQUENTIAL);

            Rack * midirack = new Rack(Rack::SEQUENTIAL);
            for (const auto & m: midi_effects)
                if (m != "Scale") midirack->add(static_cast<AMG *>(DeviceFactory::instantiate(MIDI_FX, m)));
            track->add(midirack);

            track->add(static_cast<AMG*>(DeviceFactory::instantiate(INSTRUMENT, i)));

            Rack * audiorack = new Rack(Rack::SELECTIVE);
            for (const auto & a: audio_effects)
                if (a != "Plateau") audiorack->add(static_cast<AMG*>(DeviceFactory::instantiate(AUDIO_FX, a)));
            track->add(audiorack);
            tracks_->add(track);
        }
    }



    Rack * track = new Rack(Rack::SEQUENTIAL);

    Rack * midirack = new Rack(Rack::SEQUENTIAL);
    for (const auto & m: midi_effects)
        if (m != "Scale") midirack->add(static_cast<AMG *>(DeviceFactory::instantiate(MIDI_FX, m)));
    track->add(midirack);

    track->add(static_cast<AMG*>(DeviceFactory::instantiate(INSTRUMENT, "MicInput")));

    Rack * audiorack = new Rack(Rack::SELECTIVE);
    for (const auto & a: audio_effects)
        if (a != "Plateau") audiorack->add(static_cast<AMG*>(DeviceFactory::instantiate(AUDIO_FX, a)));
    track->add(audiorack);
    tracks_->add(track);







    std::string base_path = "../res/samples";
    auto dirs = list_dir(base_path.c_str());
    for (auto & dir : dirs) {
        try {
            Rack * track = new Rack(Rack::SEQUENTIAL);

            Rack * midirack = new Rack(Rack::SEQUENTIAL);
            for (const auto & m: midi_effects)
                if (m != "Scale") midirack->add(static_cast<AMG*>(DeviceFactory::instantiate(MIDI_FX, m)));
            track->add(midirack);

            std::string samplepack_path = base_path.append("/") + dir;
            auto files = list_dir(samplepack_path.c_str());
            int nfiles = 0;
            for (auto &f: files)
                if (f.find(".wav") != std::string::npos) nfiles++;
            if (nfiles > 1) {
                auto kit = new SampleKit();
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
                if (valid) {
                    track->add(kit);
                    tracks_->add(track);
                }
            } else if (nfiles == 1) {
                for (auto &f: files) {
                    if (f.find(".wav") == std::string::npos) continue;
                    std::string notenum = f.substr(f.find("_") + 1, std::string::npos);
                    notenum = notenum.substr(0, notenum.find("."));
                    int8_t note = std::stoi(notenum);
                    std::string sample_path = samplepack_path.append("/") + f;
                    auto sampler = new Sampler();
                    sampler->init(sample_path.c_str(), note);
                    sampler->setName(dir.c_str());
                    track->add(sampler);
                    tracks_->add(track);
                    break;
                }
            }

            Rack * audiorack = new Rack(Rack::SELECTIVE);
            for (const auto & a: audio_effects)
                if (a != "Plateau") audiorack->add(static_cast<AMG*>(DeviceFactory::instantiate(AUDIO_FX, a)));
            track->add(audiorack);
        } catch (...) {
            std::cout << "unable to open pack " << dir << std::endl;
        }
    }

    return tracks_;
}

void DAW::midiOut(std::deque<MData> &q, Sync & sync) {
    master_midi_fx->midiOut(midiOutQueue, sync);
    tracks->midiOut(midiOutQueue, sync);
    tapes->midiOut(midiOutQueue, sync);
    midiOutQueue.clear();
}

void DAW::save() {
    dawState = SAVING;

    tinyxml2::XMLDocument * xmlDoc = new tinyxml2::XMLDocument();

    tinyxml2::XMLElement * root = xmlDoc->NewElement("LoopaProject");
    xmlDoc->InsertFirstChild(root);

    tinyxml2::XMLElement * projectSettings = xmlDoc->NewElement("ProjectSettings");
    projectSettings->SetAttribute("num_samples", 0);
    projectSettings->SetAttribute("root_directory", "../res/projects/current_project");
    root->InsertFirstChild(projectSettings);

    tinyxml2::XMLElement * instrumentList = xmlDoc->NewElement("Instruments");
    root->InsertEndChild(instrumentList);

    for (int i = 0; i < tracks->get_size(); i++) {
        tinyxml2::XMLElement * instrumentSettings = xmlDoc->NewElement(("I" + std::to_string(i)).c_str());
        instrumentList->InsertEndChild(instrumentSettings);

        tinyxml2::XMLElement * midiFXState = xmlDoc->NewElement("MIDI_FX");
        auto midiRack = (Rack*)(((Rack*)tracks->get_item(i))->get_item(0));
        for (int j = 0; j < midiRack->get_size(); j ++) {
            tinyxml2::XMLElement * deviceState = xmlDoc->NewElement("DEVICE");
            auto device = midiRack->get_item(j);
            deviceState->SetAttribute("classname", device->getClassName());
            device->save(xmlDoc, deviceState);
            midiFXState->InsertEndChild(deviceState);
        }
        instrumentSettings->InsertEndChild(midiFXState);

        tinyxml2::XMLElement * deviceState = xmlDoc->NewElement("DEVICE");
        instrumentSettings->InsertEndChild(deviceState);
        auto device = ((Rack*)tracks->get_item(i))->get_item(1);
        std::string element_name = device->getClassName();
        element_name += "_" + std::to_string(i);
        deviceState->SetAttribute("classname", device->getClassName());
        device->save(xmlDoc, deviceState);

        tinyxml2::XMLElement * audioFXState = xmlDoc->NewElement("AUDIO_FX");
        auto audioRack = (Rack*)(((Rack*)tracks->get_item(i))->get_item(2));
        for (int j = 0; j < audioRack->get_size(); j ++) {
            tinyxml2::XMLElement * deviceState = xmlDoc->NewElement("DEVICE");
            auto device = audioRack->get_item(j);
            deviceState->SetAttribute("classname", device->getClassName());
            device->save(xmlDoc, deviceState);
            audioFXState->InsertEndChild(deviceState);
        }
        instrumentSettings->InsertEndChild(audioFXState);
    }

    tinyxml2::XMLElement * recorderState = xmlDoc->NewElement("Recorder");
    tapes->save(xmlDoc, recorderState);
    root->InsertEndChild(recorderState);

    xmlDoc->SaveFile("../res/projects/current_project/project.loop");

    std::cout << "project saved" << std::endl;

    dawState = NORMAL;
}

void DAW::load() {
    dawStateMutex.lock();
    dawState = LOADING;
    dawStateMutex.unlock();

    tinyxml2::XMLDocument xmlDoc;
    xmlDoc.LoadFile("../res/projects/current_project/project.loop");

    tinyxml2::XMLNode * root = xmlDoc.FirstChild();
    tinyxml2::XMLElement * instrumentsList = root->FirstChildElement("Instruments");

    SCREEN_IDX = SCREENS::TAPE_VIEW;
//    if (tracks != nullptr) tracks.clear();
    delete tracks;
    delete tapes;
    delete master_midi_fx;
    delete master_audio_fx;

    master_midi_fx = new Rack(Rack::SEQUENTIAL);
    master_scale = new Scale();
    master_midi_fx->add(master_scale);
    master_midi_fx->add(new Arpeggiator());

    tracks = new Rack(Rack::SELECTIVE);
    tapes = new Tape();
    master_audio_fx = tapes->master_effects;

    auto instrumentState = instrumentsList->FirstChild();
    while (instrumentState != nullptr) {
        Rack * track = new Rack(Rack::SEQUENTIAL);

        Rack * midirack = new Rack(Rack::SEQUENTIAL);
        auto midiList = instrumentState->FirstChild();
        auto midiFX = midiList->FirstChildElement();
        while (midiFX != nullptr) {
            const char * classname;
            midiFX->QueryAttribute("classname", &classname);
            std::string classnamestr = classname;
            midirack->add(static_cast<AMG*>(DeviceFactory::instantiate(DEVICE_TYPE::MIDI_FX, classnamestr)));
            midirack->get_back()->load(midiFX);
            midiFX = midiFX->NextSiblingElement();
        }
        track->add(midirack);

        tinyxml2::XMLElement * deviceState = midiList->NextSiblingElement();
        const char * classname;
        deviceState->QueryAttribute("classname", &classname);
        std::string classnamestr = classname;
        track->add(static_cast<AMG*>(DeviceFactory::instantiate(DEVICE_TYPE::INSTRUMENT, classnamestr)));
        track->get_back()->load(deviceState);

        Rack * audiorack = new Rack(Rack::SELECTIVE);
        auto audioList = deviceState->NextSiblingElement();
        auto audioFX = audioList->FirstChildElement();
        while (audioFX != nullptr) {
            const char * classname;
            audioFX->QueryAttribute("classname", &classname);
            std::string classnamestr = classname;
            audiorack->add(static_cast<AMG*>(DeviceFactory::instantiate(DEVICE_TYPE::AUDIO_FX, classnamestr)));
            audiorack->get_back()->load(audioFX);
            audioFX = audioFX->NextSiblingElement();
        }
        track->add(audiorack);

        tracks->add(track);
        track->dive_next();

        instrumentState = instrumentState->NextSibling();
    }

    tinyxml2::XMLElement * recorderState = root->FirstChildElement("Recorder");
    tapes->load(recorderState);

    last_midi_time = 0;
    std::cout << "project loaded" << std::endl;
    reload_mutex.lock();
    reloaded = true;
    reload_mutex.unlock();

    dawStateMutex.lock();
    dawState = NORMAL;
    dawStateMutex.unlock();
}

void DAW::read_wlan_settings() {
    std::ifstream wlan_cfg_file(wlan_cfg_path);
    if (wlan_cfg_file.fail()) {
        wlan_cfg = "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev\n"
                   "ap_scan=1\n"
                   "update_config=1\n"
                   "country=RU\n"
                   "\n"
                   "### auto hotspot ###      # has to be the first network section!\n"
                   "network={\n"
                   "    priority=0            # Lowest priority, so wpa_supplicant prefers the other networks below\n"
                   "    ssid=\"loopa\"    # your access point's name\n"
                   "    mode=2\n"
                   "    key_mgmt=WPA-PSK\n"
                   "    psk=\"loopa\"      # your access point's password\n"
                   "    frequency=2462\n"
                   "}\n"
                   "\n"
                   "network={\n"
                   "     ssid=\"loopa\"\n"
                   "     psk=\"loopa\"\n"
                   "     key_mgmt=WPA-PSK\n"
                   "}";
        simulated_cfg = true;
    } else {
        std::ostringstream text;
        text << wlan_cfg_file.rdbuf();
        wlan_cfg = text.str();
        simulated_cfg = false;
    }
}

void DAW::write_wlan_settings() {
    std::ofstream wlan_cfg_file(wlan_cfg_path);
    wlan_cfg_file << wlan_cfg;
}

void DAW::get_ssid(std::string &ssid) {
    size_t ssid_beg = wlan_cfg.rfind("ssid") + 6;
    size_t ssid_end = wlan_cfg.find("\"", ssid_beg+1);
    ssid = wlan_cfg.substr(ssid_beg, ssid_end-ssid_beg);
}

void DAW::get_pwd(std::string &pwd) {
    size_t psk_beg = wlan_cfg.rfind("psk") + 5;
    size_t psk_end = wlan_cfg.find("\"", psk_beg+1);
    pwd = wlan_cfg.substr(psk_beg, psk_end-psk_beg);
}

void DAW::set_ssid(std::string &ssid) {
    size_t ssid_beg = wlan_cfg.rfind("ssid") + 6;
    size_t ssid_end = wlan_cfg.find("\"", ssid_beg);

    wlan_cfg.replace(ssid_beg, ssid_end - ssid_beg, ssid);
    if (!simulated_cfg) write_wlan_settings();
}

void DAW::set_pwd(std::string &pwd) {
    size_t psk_beg = wlan_cfg.rfind("psk") + 5;
    size_t psk_end = wlan_cfg.find("\"", psk_beg);

    wlan_cfg.replace(psk_beg, psk_end - psk_beg, pwd);
    if (!simulated_cfg) write_wlan_settings();
}