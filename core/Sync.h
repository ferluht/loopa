//
// Created by ferluht on 17/01/2024.
//

#pragma once

class Sync
{

    float bpm;

public:
    Sync();

    inline float getBPM() {return bpm;}
    inline void setBPM(float bpm_) {bpm = bpm_;}
};