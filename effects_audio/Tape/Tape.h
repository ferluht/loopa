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

#define TAPE_NUM_CHANNELS 4

class Tape : public AudioEffect {

    const float TAPE_MAX_MINUTES = 1.5;

    const long long button_threshold_time = 250 * SAMPLERATE / 1000.0;
    long long button_pressed_timedelta = 0;
    bool transition_to_overwrite_flag = false;
    bool overwrite_triggered_flag = false;
    float overwrite_start_position = 0;
    int overwrite_copying_position = 0;

    ADSR fading_adsr;
    ADSR speedup_adsr;

    int active_channel = 0;

    int bpm = 100;

    unsigned int doubling_progress = 0;
    unsigned int doubling_size = 0;
    const unsigned int max_doubling_stepsize = 8192;
    const unsigned int max_loop_size = TAPE_MAX_MINUTES*60*SAMPLERATE;

    int32_t loop_start;
    int32_t loop_end;
    int32_t loop_size;
    float position;
    float old_position;
    float base_loop_size = -1;
    const float display_div_step = 20.0;

    float cassete_phase = 0;

    int state;
    float speed = 1.0;

    std::vector<float> regular_spline_buffer;
    std::vector<float> overwrite_spline_buffer;

    float level = 1;

    bool isplaying = false;

    std::vector<std::vector<float>> audios;

    std::vector<float> overwrite_tmp_buffer;

    float avg = 0, avg_env = 0;

    int tick_counter = 0;

    bool monitoring = true;

    WavFile<float> wf;
    int savingprogress = 0;

    std::atomic<float> amp;

    SampleKit * click;

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

    inline void incrementPosition(float & position, unsigned int nframes, float speed_) {
        position += speed_ * nframes;
        if (position >= loop_end) position -= loop_size;
        if (isnan(position)) {
            std::cout << speed_ << "\n";
        }
    }

    inline void getSampleAtPosition(std::vector<float> & audio, float position_, float & lsample, float & rsample) {
        int p0 = (int)position_ - 1;
        int p1 = (int)position_ + 0;
        int p2 = (int)position_ + 1;
        int p3 = (int)position_ + 2;
        if (p0 < loop_start) p0 = loop_size + p0;
        if (p2 >= loop_end) p2 -= loop_size;
        if (p3 >= loop_end) p3 -= loop_size;

        lsample = InterpolateHermite4pt3oX(audio[p0*2 + 0],
                                        audio[p1*2 + 0],
                                        audio[p2*2 + 0],
                                        audio[p3*2 + 0],
                                        position_ - (int)position_);

//        if (isnan(lsample)) {
//            std::cout << p0*2 << " " << p1*2 << " " << p2*2 << " " << p3*2 << "\n";
//            std::cout << audio[p0 * 2 + 0] << " " << audio[p1 * 2 + 0] << " " << audio[p2 * 2 + 0] << " "
//                      << audio[p3 * 2 + 0] << "\n";
//        }

        rsample = InterpolateHermite4pt3oX(audio[p0*2 + 1],
                                        audio[p1*2 + 1],
                                        audio[p2*2 + 1],
                                        audio[p3*2 + 1],
                                        position_ - (int)position_);
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

    inline void updateInactiveChannels(float (&outBufs)[TAPE_NUM_CHANNELS][BUF_SIZE * 2], unsigned int i, float position_) {
        float lsample, rsample;
        for (int j = 0; j < TAPE_NUM_CHANNELS; j ++) {
            if (j == active_channel) continue;
            getSampleAtPosition(audios[j], position_, lsample, rsample);
            outBufs[j][i + 0] = lsample;
            outBufs[j][i + 1] = rsample;
        }
    }

    inline void setSampleAtPosition(std::vector<float> & audio, std::vector<float> & spline_buffer,
                                    float position_, float speed_) {

        float p1 = position_ - 2;
        float p2 = position_ - 1;

        if (p1 < 0) p1 += loop_size;
        if (p2 < 0) p2 += loop_size;

        float t = ((int) p2 - p1) / speed_;
        if ((int)p2 < (int)p1) t = (loop_size - p1) / speed_;

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
                 unsigned int nBufferFrames, double streamTime) override;
    void draw(GFXcanvas1 * screen) override;
    float getPosition();
    int getState();
    float getAmp();
};