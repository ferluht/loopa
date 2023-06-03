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

class Sync {
    bool sync;
    std::unordered_map<AMG *, bool> active_devices;
    std::unordered_map<AMG *, std::function<void(void)>> callbacks;
    
public:

    Sync(){
        sync = false;
    }

    inline void attachSyncCallback(AMG * dev, std::function<void(void)> callback) { callbacks[dev] = callback; }
    inline void inactivate(AMG * dev) { active_devices.erase(dev); }
    inline void send(AMG * dev) {
        active_devices[dev] = false;
        sync = true;
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
    }
};

class Tape : public AudioEffect {

    const int TAPE_MAX_MINUTES = 6;

    float s_phase = 0;

    unsigned int doubling_progress = 0;
    unsigned int doubling_size = 0;
    const unsigned int max_doubling_stepsize = 8192;
    const unsigned int max_loop_size = TAPE_MAX_MINUTES*60*SAMPLERATE;

    int looper_state = STOP;

    unsigned long position = 0;

    float level = 1;

    std::vector<float> audio;

    float avg = 0, avg_env = 0;

    int tick_counter = 0;

    bool monitoring = true;

    Sync * sync;

    WavFile<float> wf;
    int savingprogress = 0;

public:

    enum TAPE_STATE {
        STOP,
        PLAY,
        REC,
        OVERDUB
    };

    static const float looper_ratio;

    Tape();
    Tape(Sync * sync);

    MIDISTATUS trig();
    MIDISTATUS clear();
    MIDISTATUS double_loop();
    MIDISTATUS copy(Tape * to);

//    inline float envelope(float sample, float w, float w_env) {
//        avg = w * sample + (1 - w) * avg;
//        float i = std::abs(sample - avg);
//        avg_env = w_env * i + (1 - w_env) * avg_env;
//        return avg_env;
//    }

    bool save(std::string path);

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, double streamTime) override;
    void draw(GFXcanvas1 * screen) override;
    float getPosition();
    int getState();
};