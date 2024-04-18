//
// Created by ferluht on 10/07/2022.
//

#include "NewTape.h"

Tape::Tape() : AudioEffect("TAPE") {
    amp = 0;

    input_fading_adsr.set(0.05, 0.0001, 1.0, 0.3);
    speedup_adsr.set(0.25, 0.0001, 1.0, 0.25);

    bar_size = bpmToBarSize(120);
//    setBPM(120);

    click = new SampleKit("CLK");
    click->addSample("../res/metronome/hi_60.wav", 60);
    click->addSample("../res/metronome/lo_61.wav", 61);
    click_on = false;

    master_effects = new Rack("AUDIO FX", Rack::SEQUENTIAL);
    master_effects->add(new Delay());
    master_effects->add(new Plateau());
//    master_effects->add(new Tanhx());
    master_effects->add(new DummyAudioFX());

    for (int i = 0; i < TAPE_NUM_CHANNELS; i ++) {
        for (int j = 0; j < master_effects->get_size(); j ++)
            sends[i].emplace_back("SND", 0.7);
    }

    for (int i = 0; i < 12; i ++) {
        for (int j = 0; j < TAPE_NUM_CHANNELS; j ++)
            patternsLoopBounds[i][j].set(0, bar_size * 4);
    }

    for (int i = 0; i < TAPE_NUM_CHANNELS; i ++) {
        audios.emplace_back();
        audios.back().resize(TAPE_MAX_LOOP_SIZE + 1);
        std::fill(audios.back().begin(), audios.back().end(), 0);
        channel_fading_adsrs[i].set(0.03, 0.00001, 1, 0.03);
        input_spline_buffer[i].clear();

        playheads[i].emplace_back(&patternsLoopBounds[0][i]);
        playheads[i].back().position = 0;
        playheads[i].back().fading_adsr.set(0.1, 0.00001, 1, 0.1);
        playheads[i].back().fading_adsr.reset();

        playheads[i].emplace_back(&patternsLoopBounds[0][i]);
        playheads[i].back().position = 0;
        playheads[i].back().fading_adsr.set(0.1, 0.00001, 1, 0.1);
        playheads[i].back().fading_adsr.forcePeak();
    }

//    position = 0;
//    old_position = 0;
    state = STOP;
    active_channel = 0;
    channel_fading_adsrs[active_channel].forcePeak();

    active_pattern = 0;

    addMIDIHandler(0, SCREENS::MAX_SCREENS,
                   MIDI::GENERAL::LOOP_HEADER, MIDI::GENERAL::LOOP_HEADER + 4,
                   MIDI::UI::TAPE::TRIG, MIDI::UI::TAPE::TRIG, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 == 0) return;
        channel_fading_adsrs[active_channel].gateOff();
        active_channel = cmd.status - MIDI::GENERAL::LOOP_HEADER;
        channel_fading_adsrs[active_channel].gateOn();
    });

    addMIDIHandler(0, SCREENS::MAX_SCREENS,
                   MIDI::GENERAL::LOOP_HEADER, MIDI::GENERAL::LOOP_HEADER + 4,
                   MIDI::UI::TAPE::CLEAR, MIDI::UI::TAPE::CLEAR,[this](MData &cmd, Sync &sync) -> void {
        active_channel = cmd.status - MIDI::GENERAL::LOOP_HEADER;
        if (cmd.data2 > 0) return clear();
    });

    addMIDIHandler(0, SCREENS::MAX_SCREENS,
                   MIDI::GENERAL::LOOP_HEADER, MIDI::GENERAL::LOOP_HEADER + 4,
                   MIDI::UI::TAPE::DOUBLE, MIDI::UI::TAPE::DOUBLE,[this](MData &cmd, Sync &sync) -> void {
        active_channel = cmd.status - MIDI::GENERAL::LOOP_HEADER;
        if (cmd.data2 > 0) return double_loop();
    });

    addMIDIHandler(0, SCREENS::MAX_SCREENS,
                   MIDI::GENERAL::LOOP_HEADER, MIDI::GENERAL::LOOP_HEADER + TAPE_NUM_CHANNELS,
                   MIDI::UI::TAPE::STOP, MIDI::UI::TAPE::STOP, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 > 0)
            level = std::abs(level - 1.0);
    });

    addMIDIHandler(0, SCREENS::MAX_SCREENS,
                   MIDI::GENERAL::PATTERN_HEADER, MIDI::GENERAL::PATTERN_HEADER + TAPE_NUM_CHANNELS,
                   0, 128, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 == 0) return;
        active_channel = cmd.status - MIDI::GENERAL::PATTERN_HEADER;
        changePatternTo(active_channel, cmd.data1);
    });

    addMIDIHandler({SCREENS::TAPE_VIEW}, {MIDI::GENERAL::CC_HEADER}, {CC_E1, CC_E2, CC_E3, CC_E4}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data1 == CC_E1 && cmd.data2 > 0) {
            if (encoders_local_mode) {
                playheads[active_channel].back().loopbounds->moveL(cmd.data2 - 64 > 0 ? bar_size : -bar_size);
            } else {
                for (int i = 0; i < TAPE_NUM_CHANNELS; i++)
                    playheads[i].back().loopbounds->moveL(cmd.data2 - 64 > 0 ? bar_size : -bar_size);
            }
        }
        if (cmd.data1 == CC_E2 && cmd.data2 > 0) {
            if (encoders_local_mode) {
                playheads[active_channel].back().loopbounds->moveR(cmd.data2 - 64 > 0 ? bar_size : -bar_size);
            } else {
                for (int i = 0; i < TAPE_NUM_CHANNELS; i++)
                    playheads[i].back().loopbounds->moveR(cmd.data2 - 64 > 0 ? bar_size : -bar_size);
            }
        }
        if (cmd.data1 == CC_E3 || cmd.data1 == CC_E4) {
            encoders_local_mode = cmd.data2 > 0;
        }
    });

    addMIDIHandler({SCREENS::TAPE_VIEW}, {MIDI::GENERAL::SHFT_HEADER}, {}, [this](MData &cmd, Sync &sync) -> void {
        if (cmd.data2 > 0) {
            for (int i = 0; i < TAPE_NUM_CHANNELS; i ++)
                changePatternTo(i, cmd.data1);
        } else {

        }
    });

    addMIDIHandler({}, {MIDI::GENERAL::CTRL_HEADER}, {}, [this](MData &cmd, Sync &sync) -> void {

        if (cmd.data2 == 0 && last_rising_cmd >= 0) cmd.data1 = last_rising_cmd;

        switch (cmd.data1) {
            case MIDI::UI::PLAY_BUTTON:
                if (state == REWIND) return;
                if (cmd.data2 > 0) {
                    last_rising_cmd = MIDI::UI::PLAY_BUTTON;
                } else if (last_rising_cmd >= 0) {
                    if (state != PLAY && state != STOP) {
                        state = PLAY;
                        input_fading_adsr.gateOff();
                    } else {
                        if (state == STOP) {
                            state = PLAY;
                            speedup_adsr.reset();
                            speedup_adsr.set(0.25 + ((double) rand() / (RAND_MAX)) * 0.1,
                                             0.0001, 1.0, 0.25 + ((double) rand() / (RAND_MAX)) * 0.1);
                            speedup_adsr.gateOn();
                        } else if (state == PLAY) {
                            speedup_adsr.gateOff();
                        }
                    }
                }
                break;
            case MIDI::UI::REWIND_FORWARD_BUTTON:
            case MIDI::UI::REWIND_BACKWARD_BUTTON:
                if (cmd.data2 > 0) {
                    last_rising_cmd = cmd.data1;
                    if (state == OVERDUB) state = PLAY;
                    state_mem = state;
                    if (state == PLAY) rewind_speed_offset = GLOBAL_SPEED;
                    if (state == STOP) rewind_speed_offset = 0;
                    state = REWIND;
                    rewind_target_speed = GLOBAL_SPEED * rewind_speed_factor;
                    if (cmd.data1 == MIDI::UI::REWIND_BACKWARD_BUTTON) rewind_target_speed *= -1;
                    rewind_adsr.reset();
                    rewind_adsr.set(1.5 + ((double) rand() / (RAND_MAX)) * 0.5,
                                     0.0001, 1.0, 0.5 + ((double) rand() / (RAND_MAX)) * 0.2);
                    rewind_adsr.gateOn();
                } else {
                    rewind_adsr.gateOff();
                }
                break;
            case MIDI::UI::REC_BUTTON:
                if (state == REWIND) return;
                if (cmd.data2 > 0) {
                    last_rising_cmd = MIDI::UI::REC_BUTTON;
                } else if (last_rising_cmd >= 0) {
                    if (state == STOP) {
                        speedup_adsr.reset();
                        speedup_adsr.set(0.25 + ((double) rand() / (RAND_MAX)) * 0.1,
                                         0.0001, 1.0, 0.25 + ((double) rand() / (RAND_MAX)) * 0.1);
                        speedup_adsr.gateOn();
                        input_fading_adsr.reset();
                    }
                    state = OVERDUB;
                    input_fading_adsr.gateOn();
                }
                break;
            case MIDI::UI::CLICK_BUTTON:
                if (cmd.data2 > 0) {
                    last_rising_cmd = MIDI::UI::CLICK_BUTTON;
                } else if (last_rising_cmd >= 0) {
                    click_on ^= 1;
                }
                break;
            default:
                break;
        }

        if (cmd.data2 == 0) last_rising_cmd = -1;
    });

    addDrawHandler({SCREENS::LOOP_VIEW}, [this](GFXcanvas1 * screen) -> void {
        screen->setLed(2 + active_channel, 10, 10, 10);
    });

    addDrawHandler({SCREENS::TAPE_VIEW}, [this](GFXcanvas1 * screen) -> void {
        if (encoders_local_mode) {
            screen->setTextSize(1);
            screen->setCursor(123, 7);
            screen->print("L");
        } else {
            screen->setTextSize(1);
            screen->setCursor(123, 7);
            screen->print(std::to_string(active_pattern).c_str());
        }

        screen->setLed(2 + active_channel, 10, 10, 10);

        screen->setLed(0,
                       (state == OVERDUB) * 10 * speedup_adsr.get(),
                       (state == PLAY) * 10 * speedup_adsr.get(),
                       0); //((state == OVERDUB) && isplaying) * 10);

        screen->setTextSize(1);
        screen->setCursor(5, 7);
        screen->print(std::to_string(active_channel + 1).c_str());

        screen->setCursor(5, 15);
        screen->print(std::to_string(bpm).c_str());

        if (state == REC || state == OVERDUB) screen->drawCircle(13, 5, 2, 1);
        if (state == PLAY) {
            screen->drawLine(11, 3, 15, 5, 1);
            screen->drawLine(11, 7, 15, 5, 1);
            screen->drawFastVLine(11, 3, 4, 1);
        }
        if (state == REWIND && rewind_target_speed > 0) {
            screen->drawLine(10, 3, 12, 5, 1);
            screen->drawLine(10, 7, 12, 5, 1);
            screen->drawLine(13, 3, 15, 5, 1);
            screen->drawLine(13, 7, 15, 5, 1);
            screen->setLed(1, rewind_adsr.get() * 10, rewind_adsr.get() * 10, 0);
        }
        if (state == REWIND && rewind_target_speed < 0) {
            screen->drawLine(10, 5, 12, 3, 1);
            screen->drawLine(10, 5, 12, 7, 1);
            screen->drawLine(13, 5, 15, 3, 1);
            screen->drawLine(13, 5, 15, 7, 1);
            screen->setLed(0, rewind_adsr.get() * 10, rewind_adsr.get() * 10, 0);
        }
        if (state == STOP) {
            screen->drawRect(10, 3, 5, 5, 1);
        }

        int central_point = 32;

        screen->drawLine(central_point, 17, central_point - 2, 15, 1);
        screen->drawLine(central_point, 17, central_point + 2, 15, 1);

        int offset1 = (state == REWIND || state == PLAY) ? rand() > RAND_MAX / 2 : 0;
        int offset2 = (state == REWIND || state == PLAY) ? rand() > RAND_MAX / 2 : 0;
        screen->drawRoundRect(central_point - 12, 2, 24, 14, 3, 1);
        screen->drawCircle(central_point - 6, 8 + offset1, 4, 1);
        screen->drawCircle(central_point + 5, 8 + offset2, 4, 1);

        screen->drawCircle(central_point - 6, 8 + offset1, 1, 1);
        screen->drawCircle(central_point + 5, 8 + offset2, 1, 1);

        float cos1 = std::cos(cassete_phase);
        float sin1 = std::sin(cassete_phase);
        screen->drawLine(central_point - 6 + 2 * cos1, 8 + offset1 + 2 * sin1, central_point - 6 + 3 * cos1, 8 + offset1 + 3 * sin1, 1);

        float cos2 = std::cos(cassete_phase + 3.14);
        float sin2 = std::sin(cassete_phase + 3.14);
        screen->drawLine(central_point + 5 + 2 * cos2, 8 + offset2 + 2 * sin2, central_point + 5 + 3 * cos2, 8 + offset2 + 3 * sin2, 1);

        float position = playheads[active_channel].back().position;
        cassete_phase = (position / SAMPLERATE / 2 - (int)(position / SAMPLERATE / 2)) * M_PI * 2;
        if (cassete_phase > M_PI * 2) cassete_phase = 0;

        screen->setTextSize(1);

        const float display_bar_px = 15;

        if (bar_size < TAPE_MAX_LOOP_SIZE) {
            float pos_px = position / bar_size * display_bar_px;
            for (int i = (int) ((pos_px - central_point - 20) / display_bar_px) * display_bar_px;
                 i < pos_px + (screen->width() - central_point + 20); i += display_bar_px) {
                if (i < 0) continue;
                float vline_pos = central_point - pos_px + i;
                screen->drawFastVLine(vline_pos, 18, 4, 1);
                int num = (position - (pos_px - i) / display_bar_px * bar_size) / bar_size + 0.5;
                screen->setCursor(vline_pos - 1, 29);
                screen->print(std::to_string(num).c_str());
            }

            LoopBounds * lb = playheads[active_channel].back().loopbounds;
            int los_px = lb->l / bar_size * display_bar_px;
            int loe_px = lb->r / bar_size * display_bar_px;
            int low_px = lb->size / bar_size * display_bar_px;

            screen->drawFastHLine(central_point - pos_px + los_px, 20, low_px, 1);

            screen->drawFastHLine(central_point - pos_px, 18, 32000, 1);
        } else {
            float pos_px = 0;

            LoopBounds * lb = playheads[active_channel].back().loopbounds;
            int los_px = lb->l / bar_size * display_bar_px;
            int loe_px = lb->r / bar_size * display_bar_px;
            int low_px = lb->size / bar_size * display_bar_px;

            screen->drawFastHLine(central_point - pos_px + los_px, 20, low_px, 1);
            screen->drawFastHLine(central_point - pos_px, 18, 32000, 1);
        }
    });

    clear();
}

void Tape::setBPM(float bpm_) {

    float old_bar_size = bar_size;
    bar_size = bpmToBarSize(bpm_);

    for (int i = 0; i < TAPE_NUM_CHANNELS; i ++) {
        LoopBounds * lb = playheads[i].back().loopbounds;
        int lstart = lb->l / old_bar_size;
        int lsize = lb->size / old_bar_size;

        lb->set(lstart * bar_size, (lstart + lsize) * bar_size);
    }

    bpm = bpm_;
}

void Tape::clear() {
    LoopBounds * lb = playheads[active_channel].back().loopbounds;
    std::fill(audios[active_channel].begin() + lb->l * 2,
              audios[active_channel].begin() + lb->r * 2, 0);
}

float Tape::getAmp() {
    return amp;
}

void Tape::double_loop() {
//    if (doubling_progress == 0)
//        doubling_size = audios[current_scene].size();
//    if (doubling_size * 2 > max_loop_size) {
//        return MIDISTATUS::DONE;
//    }
//    auto beg = audios[current_scene].begin() + doubling_progress;
//    unsigned int step_size = std::min(doubling_size - doubling_progress, max_doubling_stepsize);
//    auto end = beg + step_size;
//    std::copy(beg, end, std::back_inserter(audios[current_scene]));
//    doubling_progress += step_size;
//    if (doubling_progress == doubling_size) {
//        doubling_progress = 0;
//        return MIDISTATUS::DONE;
//    }
//    return MIDISTATUS::WAITING;
}

void Tape::copy(Tape * to, int to_scene) {
//    if (doubling_progress == 0)
//        doubling_size = audios[current_scene].size();
//    auto beg = audios[current_scene].begin() + doubling_progress;
//    unsigned int step_size = std::min(doubling_size - doubling_progress, max_doubling_stepsize);
//    auto end = beg + step_size;
//    std::copy(beg, end, std::back_inserter(to->audios[to_scene]));
//    doubling_progress += step_size;
//    if (doubling_progress == doubling_size) {
//        doubling_progress = 0;
//        return MIDISTATUS::DONE;
//    }
//    return MIDISTATUS::WAITING;
}

//void Tape::copy_status(int to_scene) {
//    positions[to_scene] = positions[current_scene];
//    looper_states[to_scene] = looper_states[current_scene];
//}

int Tape::getState() {
    return state;
}

unsigned int Tape::bpmToBarSize(float bpm_) {
    return SAMPLERATE * (60.0 / bpm_ * 4);
}

float Tape::barSizeToBPM(unsigned int bar_size_) {
    return (float)bar_size_ / SAMPLERATE / 240;
}

void Tape::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, Sync & sync) {

    setBPM(sync.getBPM());

    std::vector<float> & audio = audios[active_channel];
    int & looper_state = state;
    float ac_begin_position = playheads[active_channel].back().position;

    speed = GLOBAL_SPEED;

    float outBufs[TAPE_NUM_CHANNELS][BUF_SIZE * 2] = {0};
    float input_fading[BUF_SIZE];
    float speeds[BUF_SIZE];
    float channel_fading[BUF_SIZE] = {0};

    float lsample, rsample;

    unsigned int rewind_frames_processed = 0;

    if (state == REWIND) {
        for (rewind_frames_processed = 0; rewind_frames_processed < nBufferFrames; rewind_frames_processed++) {
            speeds[rewind_frames_processed] = rewind_speed_offset + rewind_adsr.getAndProcess() * (rewind_target_speed - rewind_speed_offset);
            if (rewind_adsr.ended()) {
                state = state_mem;
                break;
            }
        }
        for (int j = 0; j < TAPE_NUM_CHANNELS; j ++) {
            for (auto & ph : playheads[j]) {
                if (ph.fading_adsr.ended()) continue;
                for (unsigned int i = 0, p = 0; i < 2 * rewind_frames_processed; i += 2, p++) {
                    float fade = ph.fading_adsr.getAndProcess();
                    ph.updatePosition(1, speeds[p], true);
                    readSample(audios[j], ph, lsample, rsample);
                    outBufs[j][i + 0] += lsample * fade;
                    outBufs[j][i + 1] += rsample * fade;
                    if (ph.fading_adsr.ended()) break;
                }
            }
        }
    }

    unsigned int ovp_frames_processed = rewind_frames_processed;

    if (state == OVERDUB || state == PLAY) {
        for (ovp_frames_processed = rewind_frames_processed; ovp_frames_processed < nBufferFrames; ovp_frames_processed++) {
            input_fading[ovp_frames_processed] = input_fading_adsr.getAndProcess();
            speeds[ovp_frames_processed] = GLOBAL_SPEED * (0.5 + 0.5 * speedup_adsr.getAndProcess());
            if (speedup_adsr.ended()) {
                state = STOP;
                break;
            }
        }

        for (int j = 0; j < TAPE_NUM_CHANNELS; j++) {

            for (unsigned int p = rewind_frames_processed; p < ovp_frames_processed; p++) {
                channel_fading[p] = channel_fading_adsrs[j].getAndProcess();
//                if (channel_fading_adsrs[j].ended()) break;
            }

            for (auto & ph : playheads[j]) {
                if (ph.fading_adsr.ended()) continue;
                for (unsigned int i = rewind_frames_processed * 2, p = rewind_frames_processed; i < 2 * ovp_frames_processed; i += 2, p++) {
                    ph.updatePosition(1, speeds[p], true);
                    readSample(audios[j], ph, lsample, rsample);
                    float phfade = ph.fading_adsr.getAndProcess();

                    if (channel_fading[p] == 0) {
                        outBufs[j][i + 0] += lsample * level * phfade;
                        outBufs[j][i + 1] += rsample * level * phfade;
                        continue;
                    }

                    ph.updateInputBuffer(lsample + inputBuffer[i + 0] * input_fading[p] * channel_fading[p] * phfade,
                                         rsample + inputBuffer[i + 1] * input_fading[p] * channel_fading[p] * phfade);

                    if (input_fading[p] > 0) writeSample(audios[j], ph, speeds[p]);

                    outBufs[j][i + 0] += (lsample + inputBuffer[i + 0] * channel_fading[p]) * level * phfade;
                    outBufs[j][i + 1] += (rsample + inputBuffer[i + 1] * channel_fading[p]) * level * phfade;

                    if (ph.fading_adsr.ended()) break;
                }
            }
        }
    }

    if (state == STOP) {
        input_fading_adsr.process(nBufferFrames - ovp_frames_processed);
        for (int j = 0; j < TAPE_NUM_CHANNELS; j++) {
            for (unsigned int p = ovp_frames_processed; p < nBufferFrames; p++) {
                channel_fading[p] = channel_fading_adsrs[j].getAndProcess();
//                if (channel_fading_adsrs[j].ended()) break;
            }

            for (auto & ph : playheads[j]) {
                if (ph.fading_adsr.ended()) continue;
                for (unsigned int i = ovp_frames_processed * 2; i < 2 * nBufferFrames; i += 2) {
                    ph.updateInputBuffer(inputBuffer[i + 0], inputBuffer[i + 1]);
                }
            }

            for (unsigned int i = ovp_frames_processed * 2, p = ovp_frames_processed; i < 2 * nBufferFrames; i += 2, p++) {
                outBufs[j][i + 0] = inputBuffer[i + 0] * channel_fading[p];
                outBufs[j][i + 1] = inputBuffer[i + 1] * channel_fading[p];
            }
        }
    }

    float emptybuf[BUF_SIZE * 2];
    float j = -1;
    for (unsigned int i=0; i<2*nBufferFrames; i+=2 ) {
        emptybuf[i + 0] = 0.0001f * j;
        emptybuf[i + 1] = 0.0001f * j;
        j *= -1;
    }

    for (int e = 0; e < master_effects->get_size(); e ++) {
        for (int i = 0; i < TAPE_NUM_CHANNELS; i++)
            for (unsigned int k = 0; k < 2 * nBufferFrames; k++) {
                emptybuf[k] += outBufs[i][k] * sends[i][e].value;
                outputBuffer[k] += outBufs[i][k] * (1 - sends[i][e].value);
            }

        master_effects->get_item(e)->process(emptybuf, emptybuf, nBufferFrames, sync);

        for (unsigned int k = 0; k < 2 * nBufferFrames; k++) {
            outputBuffer[k] += emptybuf[k];
            emptybuf[k] = 0;
        }
    }

    for (unsigned int k = 0; k < 2 * nBufferFrames; k++)
        outputBuffer[k] /= master_effects->get_size();

    if (click_on) {
        int pdiv = ac_begin_position / (bar_size / 4);
        int ndiv = playheads[active_channel].back().position / (bar_size / 4);
        if (pdiv != ndiv) {
            MData cmd;
            cmd.status = MIDI::GENERAL::NOTEON_HEADER;
            cmd.data1 = ndiv % 4 == 0 ? 60 : 61;
            cmd.data2 = 120;
            click->midiIn(cmd, sync);
        }

        click->process(outputBuffer, inputBuffer, nBufferFrames, sync);
    }
}

bool Tape::save(std::string path) {
    switch (savingprogress) {
        case 0:
            wf.setBitDepth(16);
            wf.setNumChannels(2);
            wf.setSampleRate(SAMPLERATE);
            savingprogress = 1;
            break;
        case 1:
            if (wf.setAudioBuffer(audios[active_channel], 2))
                savingprogress = 2;
            break;
        case 2:
            if (wf.save(path))
                savingprogress = 3;
            break;
        case 3:
            savingprogress = 0;
            return true;
    }
    return false;
}
