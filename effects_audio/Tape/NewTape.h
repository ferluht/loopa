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
#include <Instruments.h>
#include <Rack.h>
#include <AudioEffects.h>

#define TAPE_NUM_CHANNELS 4
#define TAPE_MAX_LENGTH_MINUTES 2.5
#define TAPE_MAX_LOOP_SIZE TAPE_MAX_LENGTH_MINUTES*60*SAMPLERATE*2

struct LoopBounds {
    int32_t l;
    int32_t r;
    int32_t size;

    void set(int32_t l_, int32_t r_) {
        l = l_;
        r = r_;
        size = r - l;
    }

    void moveL(int32_t dl) {
        l += dl;
        if (l > TAPE_MAX_LOOP_SIZE) l = TAPE_MAX_LOOP_SIZE;
        if (l < 0) l = 0;
        if (l > r) l = r;
        set(l, r);
    }

    void moveR(int32_t dr) {
        r += dr;
        if (r > TAPE_MAX_LOOP_SIZE) r = TAPE_MAX_LOOP_SIZE;
        if (r < 0) r = 0;
        if (r < l) r = l;
        set(l, r);
    }
};

class TapePlayhead {

public:

    ADSR fading_adsr;
    std::vector<float> input_buffer;
    float position;
    LoopBounds * loopbounds;

    TapePlayhead(LoopBounds *loopbounds_) : loopbounds(loopbounds_) {
        for (int i = 0; i < 8; i ++) input_buffer.push_back(0);
    }

    inline float updatePosition(unsigned int nframes, float speed_, bool keepbounds) {
        bool insideloop = true; // position >= loopbounds->l && position < loopbounds->r;
        position += speed_ * nframes;
        if (keepbounds && insideloop && position >= loopbounds->r) position -= loopbounds->size;
        if (keepbounds && insideloop && position < loopbounds->l) position += loopbounds->size;
        if (position >= TAPE_MAX_LOOP_SIZE) position = 0;
        if (position < 0) position = TAPE_MAX_LOOP_SIZE - 1;
        return position;
        // if speed = 1 position -> round
    }

    inline void updateInputBuffer(float lsample, float rsample) {
        std::rotate(input_buffer.begin(), input_buffer.begin() + 2, input_buffer.end());
        input_buffer[6] = lsample;
        input_buffer[7] = rsample;
    }

    inline void copyStateFrom(TapePlayhead & ph) {
        loopbounds = ph.loopbounds;
        position = ph.position;
        input_buffer.swap(ph.input_buffer);
    }
};

class Tape : public AudioEffect {

    const long long button_threshold_time = 250 * SAMPLERATE / 1000.0;
    long long button_pressed_timedelta = 0;

    ADSR input_fading_adsr;
    ADSR speedup_adsr;
    ADSR rewind_adsr;
    ADSR channel_fading_adsrs[TAPE_NUM_CHANNELS];

    std::list<TapePlayhead> playheads[TAPE_NUM_CHANNELS];

    bool encoders_local_mode = false;

    int active_channel;
    int active_pattern;

    int bpm = 100;

    unsigned int doubling_progress = 0;
    unsigned int doubling_size = 0;
    const unsigned int max_doubling_stepsize = 8192;
    const float rewind_speed_factor = 5;

    float rewind_target_speed = 0;
    float rewind_speed_offset = 0;

//    int32_t loop_start;
//    int32_t loop_end;
//    int32_t loop_size;
//    float position;
//    float old_position;
    float bar_size = -1;

    float cassete_phase = 0;

    int state;
    int state_mem;
    float speed = 1.0;

    bool click_on = false;

    std::vector<float> input_spline_buffer[TAPE_NUM_CHANNELS];

    float level = 1;

    std::vector<std::vector<float>> audios;

    std::vector<float> overwrite_tmp_buffer;

    LoopBounds patternsLoopBounds[12][TAPE_NUM_CHANNELS];

    float avg = 0, avg_env = 0;

    int tick_counter = 0;

    bool monitoring = true;

    WavFile<float> wf;
    int savingprogress = 0;

    std::atomic<float> amp;

    SampleKit * click;
    std::vector<Parameter> sends[TAPE_NUM_CHANNELS];

    int last_rising_cmd = -1;

    unsigned int bpmToBarSize(float bpm_);
    float barSizeToBPM(unsigned int bar_size_);

public:

    Rack * master_effects;

    enum TAPE_STATE {
        STOP,
        PLAY,
        REC,
        OVERDUB,
        OVERWRITE,
        REWIND
    };

    static const float looper_ratio;

    Tape();

    void clear();
    void double_loop();
    void copy(Tape * to, int to_scene);
    void copy_status(int to_scene);

    void select_scene(int s);

    inline void set_speed(float s) {
        speed = s;
    }

    inline void changePatternTo(int channel, int pattern) {
        auto & ph_old = playheads[channel].back();
        auto & ph_new = playheads[channel].front();

        active_pattern = pattern;

        ph_new.copyStateFrom(ph_old);
        ph_new.fading_adsr.forcePeak();
        ph_new.fading_adsr.gateOff();

        ph_old.loopbounds = &patternsLoopBounds[active_pattern][channel];
        ph_old.fading_adsr.reset();
        ph_old.fading_adsr.gateOn();
    }

//    inline float incrementPosition(float position, unsigned int nframes, float speed_) {
//        position += speed_ * nframes;
//        if (position >= loop_end) position -= loop_size;
//        if (position < loop_start) position += loop_size;
//        return position;
//        // if speed = 1 position -> round
//    }

    static inline void readSample(std::vector<float> & audio, TapePlayhead & ph,
                                  float & lsample, float & rsample) {
        int position = (int)ph.position;
        LoopBounds * lb = ph.loopbounds;

        int p0 = position - 1;
        int p1 = position + 0;
        int p2 = position + 1;
        int p3 = position + 2;
        if (p0 < lb->l) p0 = lb->size + p0;
        if (p2 >= lb->r) p2 -= lb->size;
        if (p3 >= lb->r) p3 -= lb->size;

        lsample = InterpolateHermite4pt3oX(audio[p0*2 + 0],
                                        audio[p1*2 + 0],
                                        audio[p2*2 + 0],
                                        audio[p3*2 + 0],
                                        ph.position - (float)position);

        rsample = InterpolateHermite4pt3oX(audio[p0*2 + 1],
                                        audio[p1*2 + 1],
                                        audio[p2*2 + 1],
                                        audio[p3*2 + 1],
                                        ph.position - (float)position);
    }

    static inline void writeSample(std::vector<float> & audio, TapePlayhead & ph, float speed_) {
        LoopBounds * lb = ph.loopbounds;
        auto & spline_buffer = ph.input_buffer;

        float p1 = ph.position - 2;
        float p2 = ph.position - 1;

        if (p1 < 0) p1 += lb->size;
        if (p2 < 0) p2 += lb->size;

        float t = ((int) p2 - p1) / speed_;
        if ((int)p2 < (int)p1) t = (lb->size - p1) / speed_;

        if ((int)p1 != (int)p2) {
            float lsample_ = InterpolateHermite4pt3oX(spline_buffer[0],
                                               spline_buffer[2],
                                               spline_buffer[4],
                                               spline_buffer[6],
                                               t);

            float rsample_ = InterpolateHermite4pt3oX(spline_buffer[1],
                                               spline_buffer[3],
                                               spline_buffer[5],
                                               spline_buffer[7],
                                               t);

            audio[(int) p2 * 2 + 0] = lsample_;
            audio[(int) p2 * 2 + 1] = rsample_;
        } else if (fabs(p2 - (int)p2) < 1e-10) {
            audio[(int) p2 * 2 + 0] = spline_buffer[2];
            audio[(int) p2 * 2 + 1] = spline_buffer[3];
        }
    }

    bool save(std::string path);

    void process(float *outputBuffer, float * inputBuffer,
                 unsigned int nBufferFrames, Sync & sync) override;
    float getPosition();
    int getState();
    float getAmp();

    void setBPM(float bpm);
};