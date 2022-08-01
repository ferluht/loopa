#pragma once
#include "SampleDelay.hpp"

#define depth 3

class ExpFilter {

public:

    float freq = 4000;
    float sum = 0;
    float coeffIn = freq / 10000;
    float coeffDel = 1 - coeffIn;
    float res = 0;

    OneSampleDelay* delayArray[depth];

    float delayCounter = 0;
    float cv = 0;
    float in = 0;
    float out = 0;
    float buf = 0;
    float lp = 0;
    float hp = 0;

  ExpFilter() {
    for (int i = 0; i < depth; i++) {
      delayArray[i] = new OneSampleDelay();
    }
  }

  void process () {
    coeffIn = freq / 10000;
    if (coeffIn > 1) coeffIn = 0.999;

    coeffDel = 1 - coeffIn;
    out = 0;

    delayArray[0]->in = in * coeffIn + delayArray[0]->out * coeffDel - delayArray[depth - 1]->out * res * coeffIn;
    out = delayArray[0]->out;
    for (int i = 1; i < depth; i++) {
      delayArray[i]->in = out * coeffIn + delayArray[i]->out * coeffDel;
      out = delayArray[i]->out;
    }
    for (int i = 0; i < depth; i++) {
      delayArray[i]->process();
    }

    in = out;
    delayArray[0]->in = in * coeffIn + delayArray[0]->out * coeffDel - delayArray[depth - 1]->out * res * coeffIn;
    out = delayArray[0]->out;
    for (int i = 1; i < depth; i++) {
      delayArray[i]->in = out * coeffIn + delayArray[i]->out * coeffDel;
      out = delayArray[i]->out;
    }
    for (int i = 0; i < depth; i++) {
      delayArray[i]->process();
    }
  }
};