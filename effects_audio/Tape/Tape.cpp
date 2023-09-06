//
// Created by ferluht on 10/07/2022.
//

#include "Tape.h"

Tape::Tape() : Tape(nullptr){}

Tape::Tape(Sync * sync) : AudioEffect("TAPE") {
    amp = 0;

    if (sync) this->sync = sync;
    else this->sync = new Sync();

    fading_adsr.set(0.01, 0.005, 1.0, 7);

    for (int i = 0; i < scenes; i ++) {
        audios.emplace_back();
        audios.back().reserve(max_loop_size);
        looper_states.push_back(STOP);
        positions.push_back(0);
        old_positions.push_back(0);
    }

    sync->attachSyncCallback(this, [this](){ trig(127); });

    addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::TAPE::TRIG, [this](MData &cmd) -> MIDISTATUS {
        return trig(cmd.data2);
        return MIDISTATUS::DONE;
    });

    addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::TAPE::CLEAR, [this](MData &cmd) -> MIDISTATUS {
        if (cmd.data2 > 0) return clear();
        return MIDISTATUS::DONE;
    });

    addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::TAPE::DOUBLE, [this](MData &cmd) -> MIDISTATUS {
        if (cmd.data2 > 0) return double_loop();
        return MIDISTATUS::DONE;
    });

    addMIDIHandler(MIDI::GENERAL::CC_HEADER, MIDI::UI::TAPE::STOP, [this](MData &cmd) -> MIDISTATUS {
//        looper_state = STOP;
        if (cmd.data2 > 0)
            level = std::abs(level - 1.0);
//        position = 0;
        return MIDISTATUS::DONE;
    });

    clear();
}

void Tape::select_scene(int s) {
    if (looper_states[current_scene] != STOP)
        looper_states[current_scene] = PLAY;
    previous_scene = current_scene;
    current_scene = s;
}

MIDISTATUS Tape::trig(uint8_t v) {
    int & looper_state = looper_states[current_scene];
    std::vector<float> & audio = audios[current_scene];

    switch (looper_state) {
        case STOP:
            if (v > 0) {
                if (sync->wait(this)) break;
                if (!audio.empty()) looper_state = PLAY;
                else {
                    looper_state = REC;
                    fading_adsr.reset();
                    fading_adsr.gateOn();
                }
                regular_spline_buffer.clear();
            }
            break;
        case REC:
            if (v > 0) {
                if (sync->wait(this)) break;
                looper_state = OVERDUB;
            }
            break;
        case OVERWRITE:
            if (v == 0) {
                if (button_pressed_timedelta > button_threshold_time) {
                    fading_adsr.gateOff();
                    looper_state = PLAY;
                } else {
                    transition_to_overwrite_flag = false;
                    overwrite_tmp_buffer.clear();
                    overwrite_copying_position = 0;
                    looper_state = OVERDUB;
                }
            }
            break;
        case OVERDUB:
            if (v > 0) {
                looper_state = PLAY;
                fading_adsr.gateOff();
            }
            break;
        case PLAY:
            if (v > 0) {
                fading_adsr.reset();
                fading_adsr.gateOn();
                overwrite_triggered_flag = true;
                transition_to_overwrite_flag = true;
                looper_state = OVERWRITE;
                button_pressed_timedelta = 0;
                overwrite_spline_buffer.clear();
            }
            break;
        default:
            break;
    }
    return MIDISTATUS::DONE;
}

MIDISTATUS Tape::clear() {
    if (looper_states[current_scene] == PLAY) {
        looper_states[current_scene] = STOP;
        audios[current_scene].clear();
        sync->inactivate(this);
        positions[current_scene] = 0;
        avg = 0;
        speed = 1;
        avg_env = 0;
    } else {
        std::fill(audios[current_scene].begin(), audios[current_scene].end(), 0);
    }
    fading_adsr.reset();
    fading_adsr.gateOn();
    return MIDISTATUS::DONE;
}

float Tape::getAmp() {
    return amp;
}

MIDISTATUS Tape::double_loop() {
    if (doubling_progress == 0)
        doubling_size = audios[current_scene].size();
    if (doubling_size * 2 > max_loop_size) {
        return MIDISTATUS::DONE;
    }
    auto beg = audios[current_scene].begin() + doubling_progress;
    unsigned int step_size = std::min(doubling_size - doubling_progress, max_doubling_stepsize);
    auto end = beg + step_size;
    std::copy(beg, end, std::back_inserter(audios[current_scene]));
    doubling_progress += step_size;
    if (doubling_progress == doubling_size) {
        doubling_progress = 0;
        return MIDISTATUS::DONE;
    }
    return MIDISTATUS::WAITING;
}

MIDISTATUS Tape::copy(Tape * to, int to_scene) {
    if (doubling_progress == 0)
        doubling_size = audios[current_scene].size();
    auto beg = audios[current_scene].begin() + doubling_progress;
    unsigned int step_size = std::min(doubling_size - doubling_progress, max_doubling_stepsize);
    auto end = beg + step_size;
    std::copy(beg, end, std::back_inserter(to->audios[to_scene]));
    doubling_progress += step_size;
    if (doubling_progress == doubling_size) {
        doubling_progress = 0;
        return MIDISTATUS::DONE;
    }
    return MIDISTATUS::WAITING;
}

void Tape::copy_status(int to_scene) {
    positions[to_scene] = positions[current_scene];
    looper_states[to_scene] = looper_states[current_scene];
}

float Tape::InterpolateHermite4pt3oX(float x0, float x1, float x2, float x3, float t)
{
    float c0 = x1;
    float c1 = .5F * (x2 - x0);
    float c2 = x0 - (2.5F * x1) + (2 * x2) - (.5F * x3);
    float c3 = (.5F * (x3 - x0)) + (1.5F * (x1 - x2));
    return (((((c3 * t) + c2) * t) + c1) * t) + c0;
}

void Tape::process(float *outputBuffer, float *inputBuffer, unsigned int nBufferFrames, double streamTime) {

    float local_amp = 0;

    float & position = positions[current_scene];
    std::vector<float> & audio = audios[current_scene];
    int & looper_state = looper_states[current_scene];
    for (int i = 0; i < scenes; i ++) old_positions[i] = positions[i];

    speed = GLOBAL_SPEED;

    if (overwrite_triggered_flag) {
        overwrite_start_position = position;
        overwrite_triggered_flag = false;
    }

    if (transition_to_overwrite_flag && button_pressed_timedelta > button_threshold_time) {
        transition_to_overwrite_flag = false;
        overwrite_spline_buffer.clear();
        for (int i = 3; i > -1; i --) {
            float p = overwrite_start_position - i * speed;
            if (p < 0) p += audio.size() / 2;
            float lsample, rsample;
            getSampleAtPosition(audio, p, lsample, rsample);
            overwrite_spline_buffer.push_back(lsample);
            overwrite_spline_buffer.push_back(rsample);
        }
    }

    button_pressed_timedelta += nBufferFrames;

    switch (looper_state) {
        case STOP:
            std::copy(inputBuffer, inputBuffer + nBufferFrames * 2, outputBuffer);
            break;
        case REC:
            for (unsigned int i = 0; i < 2*nBufferFrames; i += 2) {
                float lsample, rsample;
                float adsrval = fading_adsr.get();
                lsample = soft_clip(inputBuffer[i+0] * adsrval);
                rsample = soft_clip(inputBuffer[i+1] * adsrval);
                updateSplineBuffer(regular_spline_buffer, lsample, rsample);
                setSampleAtPosition(audio, regular_spline_buffer, position, lsample, rsample, speed, true);
                position += speed;
                fading_adsr.process();
            }
            std::copy(inputBuffer, inputBuffer + nBufferFrames * 2, outputBuffer);
            break;
        case OVERDUB:
            for (unsigned int i = 0; i < 2*nBufferFrames; i += 2) {
                float adsrval = fading_adsr.get();
                float lsample, rsample;
                getSampleAtPosition(audio, position, lsample, rsample);

                lsample = soft_clip(lsample + inputBuffer[i + 0] * adsrval);
                rsample = soft_clip(rsample + inputBuffer[i + 1] * adsrval);
                updateSplineBuffer(regular_spline_buffer, lsample, rsample);
                setSampleAtPosition(audio, regular_spline_buffer, position, lsample, rsample, speed, false);

                outputBuffer[i + 0] = lsample * level;
                outputBuffer[i + 1] = rsample * level;
                incrementPosition(position, audio, 1, speed);
                fading_adsr.process();
            }
            break;
        case OVERWRITE:
            for (unsigned int i = 0; i < 2*nBufferFrames; i += 2) {
                float adsrval = fading_adsr.get();
                float lsample, rsample;
                getSampleAtPosition(audio, position, lsample, rsample);
                outputBuffer[i + 0] = soft_clip(lsample + inputBuffer[i + 0] * adsrval);
                outputBuffer[i + 1] = soft_clip(rsample + inputBuffer[i + 1] * adsrval);
                lsample = soft_clip(lsample * (1 - adsrval) + inputBuffer[i + 0] * adsrval);
                rsample = soft_clip(rsample * (1 - adsrval) + inputBuffer[i + 1] * adsrval);
                updateSplineBuffer(regular_spline_buffer, lsample, rsample);
                if (transition_to_overwrite_flag) {
                    overwrite_tmp_buffer.push_back(lsample);
                    overwrite_tmp_buffer.push_back(rsample);
                } else {
                    setSampleAtPosition(audio, regular_spline_buffer, position, lsample, rsample, speed, false);
                    outputBuffer[i + 0] = lsample * level;
                    outputBuffer[i + 1] = rsample * level;
                }
                incrementPosition(position, audio, 1, speed);
                fading_adsr.process();
            }
            break;
        case PLAY:
            if (audio.empty()) break;
            for (unsigned int i = 0; i < 2*nBufferFrames; i += 2) {
                float adsrval = fading_adsr.get();
                float lsample, rsample;
                getSampleAtPosition(audio, position, lsample, rsample);
                updateSplineBuffer(regular_spline_buffer, lsample, rsample);
                if (adsrval > 0) {
                    setSampleAtPosition(audio, regular_spline_buffer, position,
                                        lsample + inputBuffer[i + 0] * adsrval,
                                        rsample + inputBuffer[i + 1] * adsrval,
                                        speed, false);
                }
                outputBuffer[i + 0] = soft_clip(lsample + inputBuffer[i + 0]) * level;
                outputBuffer[i + 1] = soft_clip(rsample + inputBuffer[i + 1]) * level;
                incrementPosition(position, audio, 1, speed);
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
            setSampleAtPosition(audio, overwrite_spline_buffer, overwrite_start_position, lsample, rsample, speed, false);

            incrementPosition(overwrite_start_position, audio, 1, speed);
            overwrite_copying_position += 2;
        }
    }

    for (int i = 0; i < scenes; i ++) {
        if (i == current_scene) continue;
        if (!audios[i].empty()) incrementPosition(positions[i], audios[i], nBufferFrames, speed);
    }

    for (int i = 0; i < scenes; i ++)
        if (positions[i] < old_positions[i]) {
            sync->send(this);
            break;
        }

    float m = *std::max_element(outputBuffer, outputBuffer + 2*nBufferFrames);
//    float mi = *std::min_element(outputBuffer, outputBuffer + 2*nBufferFrames);
//    float m = std::max(std::abs(mi), std::abs(ma));
    if (m > 1) m = 1;
    if (m < 0) m = 0;
    float alpha = 0.02;
    amp = m * alpha + (1 - alpha) * amp;
}

void Tape::draw(GFXcanvas1 * screen) {
//    nvgCircle(vg, 50, 16, 10);
//    nvgCircle(vg, 78, 16, 10);
//
//    nvgCircle(vg, 50, 16, 6);
//    nvgCircle(vg, 78, 16, 6);
//
//    nvgCircle(vg, 50, 16, 2.5);
//    nvgCircle(vg, 78, 16, 2.5);
//
//    nvgMoveTo(vg, 50, 26);
//    nvgLineTo(vg, 78, 26);
//
//    for (int i = 0; i < 5; i++) {
//        nvgMoveTo(vg, 50 + 3 * std::cos(phase + M_PI * 2 / 5 * i), 16 + 3 * std::sin(phase + M_PI * 2 / 5 * i));
//        nvgLineTo(vg, 50 + 10 * std::cos(phase + M_PI * 2 / 5 * i), 16 + 10 * std::sin(phase + M_PI * 2 / 5 * i));
//
//        nvgMoveTo(vg, 78 + 3 * std::cos(phase + M_PI * 2 / 5 * i), 16 + 3 * std::sin(phase + M_PI * 2 / 5 * i));
//        nvgLineTo(vg, 78 + 10 * std::cos(phase + M_PI * 2 / 5 * i), 16 + 10 * std::sin(phase + M_PI * 2 / 5 * i));
//    }
//
//    nvgRoundedRect(vg, 38, 4, 51, 24, 4);
//
//    nvgCircle(vg, 64, 22, 2);
//
//    nvgStrokeWidth(vg, 1);
//    nvgStrokeColor(vg, nvgRGB(255, 255, 255));
//    nvgStroke(vg);
//
//    phase += 0.1;
//    if (phase > M_PI * 2) phase = 0;
}

float Tape::getPosition() {
    return (float)positions[current_scene] / (float)audios[current_scene].size() * 2;
}

int Tape::getState() {
    return looper_states[current_scene];
}

bool Tape::save(std::string path) {
    if (audios[current_scene].empty()) return true;
    switch (savingprogress) {
        case 0:
            wf.setBitDepth(16);
            wf.setNumChannels(2);
            wf.setSampleRate(SAMPLERATE);
            savingprogress = 1;
            break;
        case 1:
            if (wf.setAudioBuffer(audios[current_scene], 2))
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
