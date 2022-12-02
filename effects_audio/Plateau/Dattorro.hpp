//
// This plate reverb is based upon Jon Dattorro's 1997 reverb algorithm.
//

#ifndef DATTORRO_HPP
#define DATTORRO_HPP

#include "AllpassFilter.hpp"
#include "OnePoleFilters.hpp"
#include "LFO.hpp"
#include <A.h>
//#include <iostream>

/// @private
class Dattorro {
public:
    Dattorro() : Dattorro(false) {};
    Dattorro(bool mono_);
    void process();
    void clear();
    void setTimeScale(float timeScale);
    void setPreDelay(float time);
    void unFreeze();

    float leftInput = 0;
    float rightInput = 0;
    float rightOut = 0.0;
    float leftOut = 0.0;
    float inputLowCut = 0.0;
    float inputHighCut = 10000.0;
    float reverbHighCut = 10000.0;
    float reverbLowCut = 0.0;
    float inputDiffusion1 = 0.75;
    float inputDiffusion2 = 0.625;
    float plateDiffusion1 = 0.7;
    float plateDiffusion2 = 0.5;
    float decay = 0.9999;
    float diffuseInput = 0.0;
    float size = 1;

private:

    bool mono = true;

    float _timeScale = 1.0;
    float _preDelayTime = 0.0;
    const long _kInApf1Time = 141;
    const long _kInApf2Time = 107;
    const long _kInApf3Time = 379;
    const long _kInApf4Time = 277;

    const long _kLeftApf1Time = 672;
    const long _kLeftDelay1Time = 4453;
    const long _kLeftApf2Time = 1800;
    const long _kLeftDelay2Time = 3720;

    const long _kRightApf1Time = 908;
    const long _kRightDelay1Time = 4217;
    const long _kRightApf2Time = 2656;
    const long _kRightDelay2Time = 3163;

    const long _kLeftTaps[7] = {266, 2974, 1913, 1996, 1990, 187, 1066};
    const long _kRightTaps[7] = {266, 2974, 1913, 1996, 1990, 187, 1066};
    long _scaledLeftTaps[7] = {0,0,0,0,0,0,0};
    long _scaledRightTaps[7] = {0,0,0,0,0,0,0};

    float _leftApf1Time = 0.0;
    float _leftApf2Time = 0.0;
    float _rightApf1Time = 0.0;
    float _rightApf2Time = 0.0;

    const float _kLfoExcursion = 16.0;

    const float _dattorroSampleRate = 29761.0;
    float _sampleRate = SAMPLERATE;
    float _dattorroScaleFactor = _sampleRate / _dattorroSampleRate;
    float _decay = 0.00;
    float _leftSum = 0.0;
    float _rightSum = 0.0;
    bool _freeze = false;
    OnePoleHPFilter _leftInputDCBlock;
    OnePoleHPFilter _rightInputDCBlock;
    OnePoleLPFilter _inputLpf;
    OnePoleHPFilter _inputHpf;

    InterpDelay2<float> _preDelay;

    AllpassFilter<float> _inApf1;
    AllpassFilter<float> _inApf2;
    AllpassFilter<float> _inApf3;
    AllpassFilter<float> _inApf4;
    float _tankFeed = 0.0;

    AllpassFilter<float> _leftApf1;
    InterpDelay2<float> _leftDelay1;
    OnePoleLPFilter _leftFilter;
    OnePoleHPFilter _leftHpf;
    AllpassFilter<float> _leftApf2;
    InterpDelay2<float> _leftDelay2;

    AllpassFilter<float> _rightApf1;
    InterpDelay2<float> _rightDelay1;
    OnePoleLPFilter _rightFilter;
    OnePoleHPFilter _rightHpf;
    AllpassFilter<float> _rightApf2;
    InterpDelay2<float> _rightDelay2;

    OnePoleHPFilter _leftOutDCBlock;
    OnePoleHPFilter _rightOutDCBlock;

    // Freeze Cross fade
    float _fade = 1.0;
    float _fadeTime = 0.002;
    float _fadeStep = 1.0 / (_fadeTime * _sampleRate);
    float _fadeDir = 1.0;

    float dattorroScale(float delayTime);
};

#endif