#pragma once
#include "ExpFilter.hpp"

/**
 * @private
 */
class SmoothCrossFade {

public:

	float a = 0;
	float b = 0;
	float c = 0;
	bool changed = false;
	int sampleCounter = 0;
	int nochangeCounter = 0;
	bool nochangeFlag = false;
	ExpFilter* filter = new ExpFilter();

	SmoothCrossFade() {
		filter->freq = 50;
	}

	void set(float v) {
		a = b; 
		sampleCounter = 0; 
		b = v; 
		changed = true;
	}

	float get() {
        return b;
	    if (changed) {
	      nochangeCounter = 0;
	      nochangeFlag = false;
	      c = ((float) sampleCounter) / (44100);
	      sampleCounter++;
	      if (c <= 1) {
	        filter->in = (b * c + a * (1 - c));
	      }
	      else {
	      	a = b;
	        filter->in = b;
	        changed = false;
	      }
	      filter->process();
	      return filter->out;
	      
	    }
	    else {
	      sampleCounter = 0;
	      if (!nochangeFlag) nochangeCounter++;
	      if (nochangeCounter > 44100) {
	        nochangeCounter = 0;
	        nochangeFlag = true;
	      }
	      a = b;
	      if (!nochangeFlag) {
	        filter->in = b;
	        filter->process();
	        return filter->out;
	      }
	      else {
	        return b;
	      }
	    }
	}
};