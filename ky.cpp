#include "ky.h"
    
    namespace ky {

float Phasor::operator()() {
    return process();
}

void Phasor::frequency(float hertz, float sampleRate) {
  frequency_ = hertz / sampleRate;
}

float Phasor::process() {
    // wrap
    if (phase_ >= 1.0f) {
      phase_ -= 1.0f;
    }
    float output = phase_ + offset_;
    if (output >= 1.0f) {
      output -= 1.0f;
    }

    phase_ += frequency_; // "side effect" // changes internal state
    return output;
}

    }