#pragma once

/**
 * @private
 */
class VCO {
public:

  float freq = 261.63;
  float sample_rate = 44100;
  float delta = M_PI * 2 / (sample_rate / freq);
  float phase = 0;

  float wave = 0;
  float amp = 1;
  float cv = 0;
  float fm = 0;
  float pw = 0.5;
  float value = 0;
  float type = 0;
  float mod = 0;
  float mod_prev = 0;
  float phase_inc = delta;
  float _alpha = 0.01;
  float out = 0;


  VCO() {
    
  } 

  void update_params() {
    type = wave / 10;
    delta = M_PI * 2 / (sample_rate / freq);
  }

  void process() {
    update_params();

    if (type >= 0) {
      value = sin(phase) * (1 - type) + (phase / M_PI - 1) * type;
    } else {
      value = sin(phase) * (1 + type) - ((phase < M_PI * pw * 2 ) * 2 - 1) * type;
    }
    
    mod = _alpha * (cv + fm) + (1 - _alpha) * mod;
    if (mod != mod_prev) { phase_inc = delta * pow(2, mod); mod_prev = mod; }
    phase += phase_inc;
    if (phase > M_PI * 2) {
      phase -= M_PI * 2;
    }
    out = value * amp;
  }
};
