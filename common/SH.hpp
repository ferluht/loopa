#pragma once

class SampleAndHold {
public:

    bool holding = false;
    float in = 0;
    float gate = 0;
    float out = 0;


  SampleAndHold() {
    
  } 

  float process() {
    if (gate > 1) {
      if (holding == false) {
        out = in;
        holding = true;
      }
    } else holding = false;
    return out;
    }
};
