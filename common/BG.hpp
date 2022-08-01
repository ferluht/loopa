#pragma once

class BernoulliGate {
public:

  float gate = 0;
  float last_gate = 0;
  char state = 'l';
  float p = 0.5;
  float out_l = 0;
  float out_r = 0;


  BernoulliGate() {
    
  } 

  void process() {
    float _rand = rand();
    if (last_gate < gate) {
      if (((float) _rand / RAND_MAX) < p)
        state = 'l';
      else
        state = 'r';  
    } 

    if (state == 'l') {
      out_l = gate;
      out_r = 0;
    } else {
      out_r = gate;
      out_l = 0;
    }

    last_gate = gate;
  }
};
