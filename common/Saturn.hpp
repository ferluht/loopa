#pragma once

/**
 * @private
 */
class Saturn {
public:

  float fold = 1;
  float sat = 1;
  float in = 0;
  float out = 0;


  Saturn() {
    
  } 

  float sigmoid(float x) {
      if (abs(x) < 1)
        return x * (1.5 - 0.5 * x * x);
      else 
        return 1 * sign(x);
  }

  float saturate(float x, float t) {
    if (abs(x)<t)
          return x;
      else
      {
          if (x > 0)
              return t + (1-t)*sigmoid((x-t)/((1-t)*1.5));
          else
              return -(t + (1-t)*sigmoid((-x-t)/((1-t)*1.5)));
      }
  }

  float sign(float x) {
    if (x < 0) {
      return -1;
    }
    else if (x > 0) {
      return 1;
    }
    else return 0;
  }

  void process() {
    out = in * (fold + 1);

    while (out > 1) {
      out = (out - 1) * -1;
      out += 1;
      while (out < 0) out *= -1;
    }
    while (out < -1) {
      out = (out + 1) * -1;
      out -= 1;
      while (out > 0) out *= -1;
    }

    out = out * sat;
    if (sat > 1)
    out = saturate(out, 0);
  }
};

