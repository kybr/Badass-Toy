#pragma once

#include <cmath>

namespace ky {

inline float map(float value, float low, float high, float Low, float High) {
  return Low + (High - Low) * ((value - low) / (high - low));
}

inline float lerp(float a, float b, float t) { return (1.0f - t) * a + t * b; }
inline float mtof(float m) { return 8.175799f * powf(2.0f, m / 12.0f); }
inline float ftom(float f) { return 12.0f * log2f(f / 8.175799f); }
inline float dbtoa(float db) { return 1.0f * powf(10.0f, db / 20.0f); }
inline float atodb(float a) { return 20.0f * log10f(a / 1.0f); }
inline float sigmoid(float x) { return 2.0f / (1.0f + expf(-x)) - 1.0f; }
// XXX softclip, etc.

template <typename F>
inline F wrap(F value, F high = 1, F low = 0) {
  jassert(high > low);
  if (value >= high) {
    F range = high - low;
    value -= range;
    if (value >= high) {
      // less common case; must use division
      value -= (F)(unsigned)((value - low) / range);
    }
  } else if (value < low) {
    F range = high - low;
    value += range;
    if (value < low) {
      // less common case; must use division
      value += (F)(unsigned)((high - value) / range);
    }
  }
  return value;
}

/// (0, 1)
inline float sin7(float x) {
    // 7 multiplies + 7 addition/subtraction
    // 14 operations
    return x * (x * (x * (x * (x * (x * (66.5723768716453 * x - 233.003319050759) + 275.754490892928) - 106.877929605423) + 0.156842000875713) - 9.85899292126983) + 7.25653181200263) - 8.88178419700125e-16;
}

// functor class
class Phasor {
  float frequency_; // normalized frequency
  float offset_;
  float phase_;

 public:
  Phasor(float hertz, float sampleRate, float offset = 0);
  float operator()();
  void frequency(float hertz, float sampleRate);
  float process();
};

class QuasiSaw {
  // variables and constants
  float osc = 0;      // output of the saw oscillator
  float phase = 0;    // phase accumulator
  float w = 0;        // normalized frequency
  float scaling = 0;  // scaling amount
  float DC = 0;       // DC compensation
  float norm = 0;              // normalization amount
  float const a0 = 2.5f;   // precalculated coeffs
  float const a1 = -1.5f;  // for HF compensation
  float in_hist = 0;           // delay for the HF filter

  float t = 0;

 public:
  void frequency(float hertz, float samplerate) {
    // calculate w and scaling
    w = hertz / samplerate;  // normalized frequency
    float n = 0.5f - w;
    scaling = 13.0f * n * n * n * n;  // calculate scaling
    DC = 0.376f - w * 0.752f;         // calculate DC compensation
    norm = 1.0f - 2.0f * w;  // calculate normalization
  }

  void virtualfilter(float t_) { t = t_; }

  float operator()() {
    // increment accumulator
    phase += 2.0f * w;
    if (phase >= 1.0f) {
      phase -= 2.0f;
    }

    // calculate next sample
    osc = (osc + sin(2 * M_PI * (phase + osc * scaling * t))) * 0.5f;

    // compensate HF rolloff
    float out = a0 * osc + a1 * in_hist;
    in_hist = osc;
    out = out + DC;  // compensate DC offset

    return out * norm;
  }
};


} // namespace ky