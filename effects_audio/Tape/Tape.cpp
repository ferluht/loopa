//
// Created by ferluht on 10/07/2022.
//

#include "Tape.h"

Tape::Tape() : AudioEffect("TAPE") {
    amp = 0;

    fading_adsr.set(0.05, 0.0001, 1.0, 0.3);
    speedup_adsr.set(0.25, 0.0001, 1.0, 0.25);

    loop_start = 0;
    loop_end = max_loop_size;
    loop_size = loop_end - loop_start;
    base_loop_size = max_loop_size;
    position = 0;

    base_loop_size = SAMPLERATE * (60.0 / (float)bpm * 4);
    loop_end = base_loop_size;
    loop_size = loop_end - loop_start;

    click = new SampleKit("CLK");
    click->addSample("../res/metronome/hi_60.wav", 60);
    click->addSample("../res/metronome/lo_61.wav", 61);

    for (int i = 0; i < TAPE_NUM_CHANNELS; i ++) {
        audios.emplace_back();
        audios.back().resize(max_loop_size + 1);
        std::fill(audios.back().begin(), audios.back().end(), 0);
    }

    position = 0;
    old_position = 0;
    state = OVERDUB;
    regular_spline_buffer.clear();

    addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::GENERAL::CC_HEADER + 4,
                   MIDI::UI::TAPE::TRIG,MIDI::UI::TAPE::TRIG, [this](MData &cmd) -> MIDISTATUS {
        active_channel = cmd.status - MIDI::GENERAL::CC_HEADER;
        return trig(cmd.data2);
    });

    addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::GENERAL::CC_HEADER + 4,
                   MIDI::UI::TAPE::CLEAR, MIDI::UI::TAPE::CLEAR,[this](MData &cmd) -> MIDISTATUS {
        active_channel = cmd.status - MIDI::GENERAL::CC_HEADER;
        if (cmd.data2 > 0) return clear();
        return MIDISTATUS::DONE;
    });

    addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::GENERAL::CC_HEADER + 4,
                   MIDI::UI::TAPE::DOUBLE, MIDI::UI::TAPE::DOUBLE,[this](MData &cmd) -> MIDISTATUS {
        active_channel = cmd.status - MIDI::GENERAL::CC_HEADER;
        if (cmd.data2 > 0) return double_loop();
        return MIDISTATUS::DONE;
    });

    addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::GENERAL::CC_HEADER + 4,
                   MIDI::UI::TAPE::STOP, MIDI::UI::TAPE::STOP, [this](MData &cmd) -> MIDISTATUS {
        if (cmd.data2 > 0)
            level = std::abs(level - 1.0);
        return MIDISTATUS::DONE;
    });

    addMIDIHandler({MIDI::GENERAL::CC_HEADER}, {CC_E1, CC_E2}, [this](MData &cmd) -> MIDISTATUS {
        if (cmd.data1 == CC_E1 && cmd.data2 > 0) {
            loop_start += (cmd.data2 - 64) * base_loop_size;
            if (loop_start < 0) loop_start = 0;
            if (loop_start > loop_end - base_loop_size) loop_start = loop_end - base_loop_size;
        }
        if (cmd.data1 == CC_E2 && cmd.data2 > 0) {
            loop_end += (cmd.data2 - 64) * base_loop_size;
            if (loop_end > max_loop_size) loop_end = max_loop_size;
            if (loop_end < loop_start + base_loop_size) loop_end = loop_start + base_loop_size;
        }
        loop_size = loop_end - loop_start;
        return MIDISTATUS::DONE;
    });

    addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::TAPE::PLAY, [this](MData &cmd) -> MIDISTATUS {
        if (cmd.data2 > 0) {
            isplaying ^= 1;

            if (isplaying) {
                if (loop_size != max_loop_size) {
                    speedup_adsr.reset();
                    speedup_adsr.gateOn();
                } else {
                    speedup_adsr.forcePeak();
                }
            } else {
                speedup_adsr.gateOff();
                fading_adsr.gateOff();
                state = PLAY;

                if (position > 0 && loop_end == max_loop_size) {
                    loop_end = position;
                    loop_size = loop_end - loop_start;
                    base_loop_size = loop_size;
                }
            }
        }

        return MIDISTATUS::DONE;
    });

    clear();
}

//void Tape::select_scene(int s) {
//    if (looper_states[current_scene] != STOP)
//        looper_states[current_scene] = PLAY;
//    previous_scene = current_scene;
//    current_scene = s;
//}

MIDISTATUS Tape::trig(uint8_t v) {
    std::vector<float> & audio = audios[active_channel];

    switch (state) {
        case OVERWRITE:
            if (v == 0) {
                if (button_pressed_timedelta > button_threshold_time) {
                    fading_adsr.gateOff();
                    state = PLAY;
                } else {
                    transition_to_overwrite_flag = false;
                    overwrite_tmp_buffer.clear();
                    overwrite_copying_position = 0;
                    state = OVERDUB;
                }
            }
            break;
        case OVERDUB:
            if (v > 0) {
                if (position > 0 && loop_end == max_loop_size) {
                    loop_end = position;
                    loop_size = loop_end - loop_start;
                    base_loop_size = loop_size;
                } else {
                    state = PLAY;
                    fading_adsr.gateOff();
                }
            }
            break;
        case PLAY:
            if (v > 0) {
//                if (isplaying) {
//                    fading_adsr.reset();
//                    fading_adsr.gateOn();
//                    overwrite_triggered_flag = true;
//                    transition_to_overwrite_flag = true;
//                    state = OVERWRITE;
//                    button_pressed_timedelta = 0;
//                    overwrite_spline_buffer.clear();
//                } else {
                    fading_adsr.reset();
                    fading_adsr.gateOn();
                    transition_to_overwrite_flag = false;
                    overwrite_tmp_buffer.clear();
                    overwrite_copying_position = 0;
                    state = OVERDUB;
//                }
            }
            break;
        default:
            break;
    }
    return MIDISTATUS::DONE;
}

MIDISTATUS Tape::clear() {
    if (state != PLAY) trig(127);
    std::fill(audios[active_channel].begin() + loop_start * 2, audios[active_channel].begin() + loop_end * 2, 0);
    return MIDISTATUS::DONE;
}

float Tape::getAmp() {
    return amp;
}

MIDISTATUS Tape::double_loop() {
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

    return MIDISTATUS::DONE;
}

MIDISTATUS Tape::copy(Tape * to, int to_scene) {
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

    return MIDISTATUS::DONE;
}

//void Tape::copy_status(int to_scene) {
//    positions[to_scene] = positions[current_scene];
//    looper_states[to_scene] = looper_states[current_scene];
//}

float Tape::InterpolateHermite4pt3oX(float x0, float x1, float x2, float x3, float t)
{
    float c0 = x1;
    float c1 = .5F * (x2 - x0);
    float c2 = x0 - (2.5F * x1) + (2 * x2) - (.5F * x3);
    float c3 = (.5F * (x3 - x0)) + (1.5F * (x1 - x2));
    return (((((c3 * t) + c2) * t) + c1) * t) + c0;
}

void Tape::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, double streamTime) {

    std::vector<float> & audio = audios[active_channel];
    int & looper_state = state;
    old_position = position;

    speed = GLOBAL_SPEED;

    float outBufs[TAPE_NUM_CHANNELS][BUF_SIZE * 2];

    if (overwrite_triggered_flag) {
        overwrite_start_position = position;
        overwrite_triggered_flag = false;
    }

    if (transition_to_overwrite_flag && button_pressed_timedelta > button_threshold_time) {
        transition_to_overwrite_flag = false;
        overwrite_spline_buffer.clear();
        for (int i = 3; i > -1; i --) {
            float p = overwrite_start_position - i * speed;
            if (p < 0) p += loop_size / 2;
            float lsample, rsample;
            getSampleAtPosition(audio, p, lsample, rsample);
            overwrite_spline_buffer.push_back(lsample);
            overwrite_spline_buffer.push_back(rsample);
        }
    }

    button_pressed_timedelta += nBufferFrames;

    if (isplaying) {
        switch (looper_state) {
            case OVERDUB:
                for (unsigned int i = 0; i < 2 * nBufferFrames; i += 2) {
                    speedup_adsr.process();
                    speed = GLOBAL_SPEED * (0.5 + speedup_adsr.get() * 0.5);

                    float adsrval = fading_adsr.get();
                    float lsample, rsample;
                    getSampleAtPosition(audio, position, lsample, rsample);

                    lsample = lsample + inputBuffer[i + 0] * adsrval;
                    rsample = rsample + inputBuffer[i + 1] * adsrval;
                    updateSplineBuffer(regular_spline_buffer, lsample, rsample);
                    setSampleAtPosition(audio, regular_spline_buffer, position, speed);

                    outBufs[active_channel][i + 0] = lsample * level;
                    outBufs[active_channel][i + 1] = rsample * level;
                    updateInactiveChannels(outBufs, i, position);
                    incrementPosition(position, 1, speed);
                    fading_adsr.process();
                }
                break;
            case OVERWRITE:
                for (unsigned int i = 0; i < 2 * nBufferFrames; i += 2) {
                    speedup_adsr.process();

                    float adsrval = fading_adsr.get();
                    float lsample, rsample;
                    getSampleAtPosition(audio, position, lsample, rsample);
                    outBufs[active_channel][i + 0] = lsample + inputBuffer[i + 0] * adsrval;
                    outBufs[active_channel][i + 1] = rsample + inputBuffer[i + 1] * adsrval;
                    lsample = lsample * (1 - adsrval) + inputBuffer[i + 0] * adsrval;
                    rsample = rsample * (1 - adsrval) + inputBuffer[i + 1] * adsrval;
                    updateSplineBuffer(regular_spline_buffer, lsample, rsample);
                    if (transition_to_overwrite_flag) {
                        overwrite_tmp_buffer.push_back(lsample);
                        overwrite_tmp_buffer.push_back(rsample);
                    } else {
                        setSampleAtPosition(audio, regular_spline_buffer, position, speed);
                        outBufs[active_channel][i + 0] = lsample * level;
                        outBufs[active_channel][i + 1] = rsample * level;
                    }
                    updateInactiveChannels(outBufs, i, position);
                    incrementPosition(position, 1, speed);
                    fading_adsr.process();
                }
                break;
            case PLAY:
                if (audio.empty()) break;
                for (unsigned int i = 0; i < 2 * nBufferFrames; i += 2) {
                    speedup_adsr.process();
                    speed = GLOBAL_SPEED * speedup_adsr.get();

                    float adsrval = fading_adsr.get();
                    float lsample, rsample;
                    getSampleAtPosition(audio, position, lsample, rsample);

                    updateSplineBuffer(regular_spline_buffer,
                                       lsample + inputBuffer[i + 0] * adsrval,
                                       rsample + inputBuffer[i + 1] * adsrval);

//                    if (adsrval > 0)
//                        setSampleAtPosition(audio, regular_spline_buffer, position, speed);

                    outBufs[active_channel][i + 0] = (lsample + inputBuffer[i + 0]) * level;
                    outBufs[active_channel][i + 1] = (rsample + inputBuffer[i + 1]) * level;

                    updateInactiveChannels(outBufs, i, position);
                    incrementPosition(position, 1, speed);
                    fading_adsr.process();
                }
                break;
        }

        if (!transition_to_overwrite_flag && !overwrite_tmp_buffer.empty()) {
            for (unsigned int i = 0; i < 2 * nBufferFrames; i += 2) {
                if (overwrite_copying_position > overwrite_tmp_buffer.size()) {
                    overwrite_copying_position = 0;
                    overwrite_tmp_buffer.clear();
                    break;
                }

                float lsample = overwrite_tmp_buffer[overwrite_copying_position + 0];
                float rsample = overwrite_tmp_buffer[overwrite_copying_position + 1];

                updateSplineBuffer(overwrite_spline_buffer, lsample, rsample);
                setSampleAtPosition(audio, overwrite_spline_buffer, overwrite_start_position, speed);

                incrementPosition(overwrite_start_position, 1, speed);
                overwrite_copying_position += 2;
            }
        }
    } else {
        for (unsigned int i = 0; i < 2 * nBufferFrames; i += 2) {
            speedup_adsr.process();
            speed = GLOBAL_SPEED * speedup_adsr.get();

            if (speed > 1e-16) {
                float adsrval = fading_adsr.get();
                float lsample, rsample;
                getSampleAtPosition(audio, position, lsample, rsample);

                updateSplineBuffer(regular_spline_buffer,
                                   lsample + inputBuffer[i + 0] * adsrval,
                                   rsample + inputBuffer[i + 1] * adsrval);

//                if (adsrval > 0)
//                    setSampleAtPosition(audio, regular_spline_buffer, position, speed);

                outBufs[active_channel][i + 0] = (lsample + inputBuffer[i + 0]) * level;
                outBufs[active_channel][i + 1] = (rsample + inputBuffer[i + 1]) * level;
                updateInactiveChannels(outBufs, i, position);
                incrementPosition(position, 1, speed);
            } else {
                updateSplineBuffer(regular_spline_buffer, inputBuffer[i + 0], inputBuffer[i + 1]);

                outBufs[active_channel][i + 0] = inputBuffer[i + 0];
                outBufs[active_channel][i + 1] = inputBuffer[i + 1];

                for (int j = 0; j < TAPE_NUM_CHANNELS; j ++) {
                    if (j == active_channel) continue;
                    outBufs[j][i + 0] = 0;
                    outBufs[j][i + 1] = 0;
                }
            }

            fading_adsr.process();
        }
    }

    int pdiv = position / (base_loop_size / 4);
    int ndiv = old_position / (base_loop_size / 4);
    if (pdiv != ndiv) {
        MData cmd;
        cmd.status = MIDI::GENERAL::NOTEON_HEADER;
        cmd.data1 = pdiv % 4 == 0 ? 60 : 61;
        cmd.data2 = 120;
        click->midiIn(cmd);
    }

    click->process(outputBuffer, inputBuffer, nBufferFrames, 0);

    for (int j = 0; j < TAPE_NUM_CHANNELS; j ++) {
        for (unsigned int i = 0; i < 2 * nBufferFrames; i += 2) {
            outputBuffer[i + 0] += outBufs[j][i + 0];
            outputBuffer[i + 1] += outBufs[j][i + 1];
        }
    }


//    float m = *std::max_element(outputBuffer, outputBuffer + 2*nBufferFrames);
//    if (m > 1) m = 1;
//    if (m < 0) m = 0;
//    float alpha = 0.02;
//    amp = m * alpha + (1 - alpha) * amp;
}

void Tape::draw(GFXcanvas1 * screen) {

    screen->fillScreen(0x00);

    screen->setTextSize(1);
    screen->setCursor(5, 7);
    screen->print(std::to_string(active_channel + 1).c_str());

    screen->setCursor(5, 15);
    screen->print(std::to_string(bpm).c_str());

    screen->setRotation(135);
    std::vector<std::string> names = {"REV", "DEL", "SAT", "VOL"};
    int p0 = 67;
    int dd = 15;
    for (int i = 0; i < 4; i ++) {
        screen->setCursor(17, p0 + 10 + dd * i);
        screen->print(names[i].c_str());
        screen->drawRect(16, p0 + dd * i, 14, 4, 1);
    }

    screen->setRotation(0);

    if (state == REC || state == OVERDUB) screen->drawCircle(13, 5, 2, 1);
    if (state == PLAY) {
        screen->drawLine(11, 3, 15, 5, 1);
        screen->drawLine(11, 7, 15, 5, 1);
        screen->drawFastVLine(11, 3, 4, 1);
    }

    int central_point = 32;

    screen->drawLine(central_point, 17, central_point - 2, 15, 1);
    screen->drawLine(central_point, 17, central_point + 2, 15, 1);

    int offset1 = isplaying ? rand() > RAND_MAX / 2 : 0;
    int offset2 = isplaying ? rand() > RAND_MAX / 2 : 0;
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

    if (isplaying) {
        cassete_phase += 0.1;
        if (cassete_phase > M_PI * 2) cassete_phase = 0;
    }

    screen->setTextSize(1);

    if (base_loop_size < max_loop_size) {
        float pos_px = position / base_loop_size * display_div_step;
        for (int i = (int) ((pos_px - central_point - 20) / display_div_step) * display_div_step;
             i < pos_px + (screen->width() - central_point + 20); i += display_div_step) {
            if (i < 0) continue;
            float vline_pos = central_point - pos_px + i;
            screen->drawFastVLine(vline_pos, 18, 4, 1);

            int num = (position - (pos_px - i) / display_div_step * base_loop_size) / base_loop_size + 0.5;

            screen->setCursor(vline_pos - 1, 29);
            screen->print(std::to_string(num).c_str());
        }

        int los_px = loop_start / base_loop_size * display_div_step;
        int loe_px = loop_end / base_loop_size * display_div_step;
        int low_px = loop_size / base_loop_size * display_div_step;

        screen->drawFastHLine(central_point - pos_px + los_px, 20, low_px, 1);

        screen->drawFastHLine(central_point - pos_px, 18, 1000000, 1);
    } else {
        float pos_px = 0;

        int los_px = loop_start / base_loop_size * display_div_step;
        int loe_px = loop_end / base_loop_size * display_div_step;
        int low_px = loop_size / base_loop_size * display_div_step;

        screen->drawFastHLine(central_point - pos_px + los_px, 20, low_px, 1);

        screen->drawFastHLine(central_point - pos_px, 18, 1000000, 1);
    }
}

bool Tape::save(std::string path) {
//    if (audios[current_scene].empty()) return true;
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
