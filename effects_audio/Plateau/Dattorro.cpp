#include <stdio.h>
#include "stdlib.h"

#include "Dattorro.hpp"

Dattorro::Dattorro(bool mono_) {
    mono = mono_;
    setTimeScale(1);
    decay = 0.9;
    inputLowCut = 440.f;
    inputHighCut = 4400.f;
    reverbLowCut = 440.f;
    reverbHighCut = 4400.f;

    _dattorroScaleFactor = _sampleRate / _dattorroSampleRate;

    _preDelay = InterpDelay2<float>(192010, 0);

    _inputLpf = OnePoleLPFilter(22000.0);
    _inputHpf = OnePoleHPFilter(0.0);

    _inApf1 = AllpassFilter<float>(dattorroScale(20 * _kInApf1Time), dattorroScale(_kInApf1Time), inputDiffusion1);
    _inApf2 = AllpassFilter<float>(dattorroScale(20 * _kInApf2Time), dattorroScale(_kInApf2Time), inputDiffusion1);
    _inApf3 = AllpassFilter<float>(dattorroScale(20 * _kInApf3Time), dattorroScale(_kInApf3Time), inputDiffusion2);
    _inApf4 = AllpassFilter<float>(dattorroScale(20 * _kInApf4Time), dattorroScale(_kInApf4Time), inputDiffusion2);

    _leftApf1 = AllpassFilter<float>(dattorroScale(40 * _kLeftApf1Time), dattorroScale(_kLeftApf1Time), -plateDiffusion1);
    _leftDelay1 = InterpDelay2<float>(dattorroScale(40 * _kLeftDelay1Time), dattorroScale(_kLeftDelay1Time));
    _leftFilter = OnePoleLPFilter(reverbHighCut);
    _leftHpf = OnePoleHPFilter(reverbLowCut);
    _leftApf2 = AllpassFilter<float>(dattorroScale(40 * _kLeftApf2Time), dattorroScale(_kLeftApf2Time), plateDiffusion2);
    _leftDelay2 = InterpDelay2<float>(dattorroScale(40 * _kLeftDelay2Time), dattorroScale(_kLeftDelay2Time));

    _rightApf1 = AllpassFilter<float>(dattorroScale(40 * _kRightApf1Time), dattorroScale(_kRightApf1Time), -plateDiffusion1);
    _rightDelay1 = InterpDelay2<float>(dattorroScale(40 * _kRightDelay1Time), dattorroScale(_kRightDelay1Time));
    _rightFilter = OnePoleLPFilter(reverbHighCut);
    _rightHpf = OnePoleHPFilter(reverbLowCut);
    _rightApf2 = AllpassFilter<float>(dattorroScale(40 * _kRightApf2Time), dattorroScale(_kRightApf2Time), plateDiffusion2);
    _rightDelay2 = InterpDelay2<float>(dattorroScale(40 * _kRightDelay2Time), dattorroScale(_kRightDelay2Time));

    _leftApf1Time = dattorroScale(_kLeftApf1Time);
    _leftApf2Time = dattorroScale(_kLeftApf2Time);
    _rightApf1Time = dattorroScale(_kRightApf1Time);
    _rightApf2Time = dattorroScale(_kRightApf2Time);

    for(auto i = 0; i < 7; ++i) {
        _scaledLeftTaps[i] = dattorroScale(_kLeftTaps[i]);
        _scaledRightTaps[i] = dattorroScale(_kRightTaps[i]);
    }

    _leftInputDCBlock.setCutoffFreq(20.0);
    _rightInputDCBlock.setCutoffFreq(20.0);
    _leftOutDCBlock.setCutoffFreq(20.0);
    _rightOutDCBlock.setCutoffFreq(20.0);

    _leftApf1.delay.setDelayTime(_leftApf1Time);
    _leftApf2.delay.setDelayTime(_leftApf2Time);
    _rightApf1.delay.setDelayTime(_rightApf1Time);
    _rightApf2.delay.setDelayTime(_rightApf2Time);

    _leftApf1.gain = -plateDiffusion1;
    _leftApf2.gain = plateDiffusion2;
    _rightApf1.gain = -plateDiffusion1;
    _rightApf2.gain = plateDiffusion2;

    _leftFilter.setCutoffFreq(reverbHighCut);
    _leftHpf.setCutoffFreq(reverbLowCut);
    _rightFilter.setCutoffFreq(reverbHighCut);
    _rightHpf.setCutoffFreq(reverbLowCut);

    setTimeScale(size);

    unFreeze();
}

void Dattorro::process() {

    if(!_freeze) {
        _decay = decay;
    }

    _leftInputDCBlock.input = leftInput;
    if (!mono) _rightInputDCBlock.input = rightInput;

    _inputLpf.setCutoffFreq(inputHighCut);
    _inputHpf.setCutoffFreq(inputLowCut);
    _inputLpf.input = _leftInputDCBlock.process() + _rightInputDCBlock.process();
    _inputHpf.input = _inputLpf.process();
    _inputHpf.process();
    _preDelay.input = _inputHpf.output;
    _preDelay.process();
    _inApf1.input = _preDelay.output;
    _inApf2.input = _inApf1.process();
    _inApf3.input = _inApf2.process();
    _inApf4.input = _inApf3.process();
    _tankFeed = _preDelay.output * (1.0 - diffuseInput) + _inApf4.process() * diffuseInput;
    _leftSum += _tankFeed;
    _rightSum += _tankFeed;

    _leftApf1.input = _leftSum;
    _leftDelay1.input = _leftApf1.process();
    _leftDelay1.process();
    _leftFilter.input = _leftDelay1.output;
    _leftHpf.input = _leftFilter.process();
    _leftApf2.input = (_leftDelay1.output * (1.0 - _fade) + _leftHpf.process() * _fade) * _decay;
    _leftDelay2.input = _leftApf2.process();
    _leftDelay2.process();

    if (!mono) {
        _rightApf1.input = _rightSum;
        _rightDelay1.input = _rightApf1.process();
        _rightDelay1.process();
        _rightFilter.input = _rightDelay1.output;
        _rightHpf.input = _rightFilter.process();
        _rightApf2.input = (_rightDelay1.output * (1.0 - _fade) + _rightHpf.process() * _fade) * _decay;
        _rightDelay2.input = _rightApf2.process();
        _rightDelay2.process();
    }

    _rightSum = _leftDelay2.output * _decay;
    if (!mono) _leftSum = _rightDelay2.output * _decay;
    else _leftSum = _rightSum;

    _leftOutDCBlock.input = _leftApf1.output;
    _leftOutDCBlock.input += _leftDelay1.tap(_scaledLeftTaps[0]);
    _leftOutDCBlock.input += _leftDelay1.tap(_scaledLeftTaps[1]);
    _leftOutDCBlock.input -= _leftApf2.delay.tap(_scaledLeftTaps[2]);
    _leftOutDCBlock.input += _leftDelay2.tap(_scaledLeftTaps[3]);

    if (!mono) {
        _leftOutDCBlock.input -= _rightDelay1.tap(_scaledLeftTaps[4]);
        _leftOutDCBlock.input -= _rightApf2.delay.tap(_scaledLeftTaps[5]);
        _leftOutDCBlock.input -= _rightDelay2.tap(_scaledLeftTaps[6]);

        _rightOutDCBlock.input = _rightApf1.output;
        _rightOutDCBlock.input += _rightDelay1.tap(_scaledRightTaps[0]);
        _rightOutDCBlock.input += _rightDelay1.tap(_scaledRightTaps[1]);
        _rightOutDCBlock.input -= _rightApf2.delay.tap(_scaledRightTaps[2]);
        _rightOutDCBlock.input += _rightDelay2.tap(_scaledRightTaps[3]);
        _rightOutDCBlock.input -= _leftDelay1.tap(_scaledRightTaps[4]);
        _rightOutDCBlock.input -= _leftApf2.delay.tap(_scaledRightTaps[5]);
        _rightOutDCBlock.input -= _leftDelay2.tap(_scaledRightTaps[6]);
    }

    leftOut = _leftOutDCBlock.process() * 0.5;
    if (!mono) rightOut = _rightOutDCBlock.process() * 0.5;
    else rightOut = leftOut;

    _fade += _fadeStep * _fadeDir;
    _fade = (_fade < 0.0) ? 0.0 : ((_fade > 1.0) ? 1.0 : _fade);

    // float[2] result = [leftOut, rightOut];
    // return result;

}

void Dattorro::clear() {
    _preDelay.clear();
    _inputLpf.clear();
    _inputHpf.clear();
    _inApf1.clear();
    _inApf2.clear();
    _inApf3.clear();
    _inApf4.clear();

    _leftApf1.clear();
    _leftDelay1.clear();
    _leftFilter.clear();
    _leftHpf.clear();
    _leftApf2.clear();
    _leftDelay2.clear();

    _rightApf1.clear();
    _rightDelay1.clear();
    _rightFilter.clear();
    _rightHpf.clear();
    _rightApf2.clear();
    _rightDelay2.clear();

    _leftInputDCBlock.clear();
    _rightInputDCBlock.clear();
    _leftOutDCBlock.clear();
    _rightOutDCBlock.clear();
}

void Dattorro::setTimeScale(float timeScale) {
    _timeScale = timeScale;
    if(_timeScale < 0.0001) {
        _timeScale = 0.0001;
    }

    _leftDelay1.setDelayTime(dattorroScale(_kLeftDelay1Time * _timeScale));
    _leftDelay2.setDelayTime(dattorroScale(_kLeftDelay2Time * _timeScale));
    _rightDelay1.setDelayTime(dattorroScale(_kRightDelay1Time * _timeScale));
    _rightDelay2.setDelayTime(dattorroScale(_kRightDelay2Time * _timeScale));
    _leftApf1Time = dattorroScale(_kLeftApf1Time * _timeScale);
    _leftApf2Time = dattorroScale(_kLeftApf2Time * _timeScale);
    _rightApf1Time = dattorroScale(_kRightApf1Time * _timeScale);
    _rightApf2Time = dattorroScale(_kRightApf2Time * _timeScale);
}

void Dattorro::setPreDelay(float t) {
    _preDelayTime = t;
    _preDelay.setDelayTime(_preDelayTime * _sampleRate);
}

void Dattorro::unFreeze() {
    _freeze = false;
    _fadeDir = 1.0;
    _decay = decay;
}

float Dattorro::dattorroScale(float delayTime) {
    return delayTime * _dattorroScaleFactor;
}
