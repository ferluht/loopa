//
// Created by ferluht on 10/07/2022.
//

#pragma once

#include <vector>
#include <unordered_map>
#include <cmath>
#include <iostream>
#include <Effect.h>
#include <WavFile.hpp>
#include <Utils.hpp>
#include <chrono>
#include <ADSR.h>
#include <algorithm>
#include <atomic>

class Sync {
    bool sync;
    int sample_counter;
    int sync_interval;
    std::unordered_map<AMG *, bool> active_devices;
    std::unordered_map<AMG *, std::function<void(void)>> callbacks;
    
public:

    Sync(){
        sync = false;
        sample_counter = 0;
    }

    inline void attachSyncCallback(AMG * dev, std::function<void(void)> callback) { callbacks[dev] = callback; }
    inline void inactivate(AMG * dev) { active_devices.erase(dev); }
    inline void send(AMG * dev) {
        active_devices[dev] = false;
        sync = true;
        if (sample_counter > 0) sync_interval = sample_counter;
        sample_counter = 0;
    }

    inline int getSyncInterval() {
        return sync_interval;
    }

    inline bool wait(AMG * dev) {
        if (!active_devices.empty()){
            if (sync)
                active_devices[dev] = false;
            else {
                active_devices[dev] = true;
                return true;
            }
        }
        return false;
    }

    inline bool isWaiting(AMG * dev) {
        auto adev = active_devices.find(dev);
        if (adev != active_devices.end() && adev->second)
            return true;
        return false;
    }

    inline void process() {
        for (auto it = callbacks.begin(); it != callbacks.end(); it ++) {
            auto dev = active_devices.find(it->first);
            if (dev != active_devices.end() && dev->second && sync)
                it->second();
        }
        sync = false;
        sample_counter ++;
    }
};

class Tape : public AudioEffect {

    const float TAPE_MAX_MINUTES = 1.5;

    const long long button_threshold_time = 150 * SAMPLERATE / 1000.0;
    long long button_pressed_timedelta = 0;
    bool transition_to_overwrite_flag = false;
    bool overwrite_triggered_flag = false;
    float overwrite_start_position = 0;
    int overwrite_copying_position = 0;

    float s_phase = 0;

    ADSR fading_adsr;

    const int scenes = 4;
    int current_scene = 0;
    int previous_scene = 0;

    unsigned int doubling_progress = 0;
    unsigned int doubling_size = 0;
    const unsigned int max_doubling_stepsize = 8192;
    const unsigned int max_loop_size = TAPE_MAX_MINUTES*60*SAMPLERATE;

    std::vector<int> looper_states;
    std::vector<float> positions;
    std::vector<float> old_positions;
    float speed = 1.0;

    std::vector<float> regular_spline_buffer;
    std::vector<float> overwrite_spline_buffer;

    float level = 1;

//    std::vector<float> audio;

    std::vector<std::vector<float>> audios;

    std::vector<float> overwrite_tmp_buffer;

    float avg = 0, avg_env = 0;

    int tick_counter = 0;

    bool monitoring = true;

    Sync * sync;

    WavFile<float> wf;
    int savingprogress = 0;

    std::atomic<float> amp;

public:

    enum TAPE_STATE {
        STOP,
        PLAY,
        REC,
        OVERDUB,
        OVERWRITE
    };

    static const float looper_ratio;

    Tape();
    Tape(Sync * sync);

    MIDISTATUS trig(uint8_t v);
    MIDISTATUS clear();
    MIDISTATUS double_loop();
    MIDISTATUS copy(Tape * to, int to_scene);
    void copy_status(int to_scene);

    void select_scene(int s);

    inline void set_speed(float s) {
        speed = s;
    }

    static float InterpolateHermite4pt3oX(float x0, float x1, float x2, float x3, float t);

    static inline void incrementPosition(float & position, std::vector<float> & audio, unsigned int nframes, float speed) {
        position += speed * static_cast<float>(nframes);
        if (position >= audio.size() / 2) position -= audio.size() / 2;
    }

    static inline void getSampleAtPosition(std::vector<float> & audio, float position, float & lsample, float & rsample) {
        int p0 = (int)position - 1;
        int p1 = (int)position + 0;
        int p2 = (int)position + 1;
        int p3 = (int)position + 2;
        if (p0 < 0) p0 = static_cast<int>(audio.size() / 2) + p0;
        if (p2 >= audio.size() / 2) p2 -= static_cast<int>(audio.size() / 2);
        if (p3 >= audio.size() / 2) p3 -= static_cast<int>(audio.size() / 2);

        lsample = InterpolateHermite4pt3oX(audio[p0*2 + 0],
                                        audio[p1*2 + 0],
                                        audio[p2*2 + 0],
                                        audio[p3*2 + 0],
                                        position - (int)position);

        rsample = InterpolateHermite4pt3oX(audio[p0*2 + 1],
                                        audio[p1*2 + 1],
                                        audio[p2*2 + 1],
                                        audio[p3*2 + 1],
                                        position - (int)position);
    }

    static inline void updateSplineBuffer(std::vector<float> & spline_buffer, float lsample, float rsample) {
        if (spline_buffer.size() == 8) {
            std::rotate(spline_buffer.begin(), spline_buffer.begin() + 2, spline_buffer.end());
            spline_buffer[6] = lsample;
            spline_buffer[7] = rsample;
        } else {
            spline_buffer.push_back(lsample);
            spline_buffer.push_back(rsample);
        }
    }

    static inline void setSampleAtPosition(std::vector<float> & audio, std::vector<float> & spline_buffer,
                                           float position, float lsample, float rsample, float speed, bool recording) {

        if (recording) {
            audio.push_back(lsample);
            audio.push_back(rsample);
        } else {
            float p1 = position - 2;
            float p2 = position - 1;

            if (p1 < 0) p1 += audio.size();
            if (p2 < 0) p2 += audio.size();

            float t = ((int) p2 - p1) / speed;
            if ((int)p2 < (int)p1) t = (audio.size() - p1) / speed;

            if ((int)p1 != (int)p2) {
                lsample = InterpolateHermite4pt3oX(spline_buffer[0],
                                                   spline_buffer[2],
                                                   spline_buffer[4],
                                                   spline_buffer[6],
                                                   t);

                rsample = InterpolateHermite4pt3oX(spline_buffer[1],
                                                   spline_buffer[3],
                                                   spline_buffer[5],
                                                   spline_buffer[7],
                                                   t);
                audio[(int) p2 * 2 + 0] = lsample;
                audio[(int) p2 * 2 + 1] = rsample;
            } else if (fabs(p2 - (int)p2) < 1e-10) {
                audio[(int) p2 * 2 + 0] = spline_buffer[2];
                audio[(int) p2 * 2 + 1] = spline_buffer[3];
            }
        }
    }

    bool save(std::string path);

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, Sync & sync) override;

    float getPosition();
    int getState();
    float getAmp();
};