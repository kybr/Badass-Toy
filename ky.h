#pragma once

#include <cmath>
#include <vector>

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

// XXX is this faster than wrap (above)?
template <typename F>
inline F wrap_fmod(F value, F high = 1, F low = 0) {
  return low + fmod(value - low, high - low);
}

class ArrayFloat : public std::vector<float> {
  public:
  float lookup(float index) { 
    int to_the_left = (int)index;
    int to_the_right = (to_the_left == (size() - 1)) ? 0 : to_the_left + 1;
    float t = index - (float)to_the_left;
    return operator[](to_the_left) * (1 - t) + t * operator[](to_the_right);
  }
  float phasor(float t) { 
    return lookup(size() * t);
  }
};

/// (0, 1)
inline float sin7(float x) {
    // 7 multiplies + 7 addition/subtraction
    // 14 operations
    return x * (x * (x * (x * (x * (x * (66.5723768716453 * x - 233.003319050759) + 275.754490892928) - 106.877929605423) + 0.156842000875713) - 9.85899292126983) + 7.25653181200263) - 8.88178419700125e-16;
}

inline float sint(float t) {
  struct TableSine : ArrayFloat {
    TableSine() {
      resize(4096);
      for (size_t i = 0; i < size(); ++i) {
        at(i) = static_cast<float>(sin((2.0 * M_PI * i) / static_cast<double>(size())));
        //printf("%f\n", at(i));
      }
    }
  };
  static TableSine table;
  return table.phasor(t);
}

// functor class
class Phasor {
  float frequency_ = 0; // normalized frequency
  float offset_ = 0;
  float phase_ = 0;

 public:
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


// std::array<type, number> ... on the stack
// std::vector<type> ... allocates memory on the heap

struct Cycle : public Phasor {
  float operator()() {
    return sint(Phasor::operator()());
  }
};

class DelayLine : public ArrayFloat {
  size_t index = 0;
  public:

  void write(float value) {
    operator[](index) = value;
    index = (index + 1) % size();
  }

  float read(float samples_ago) {
    float readIndex = (float)index - samples_ago;
    if (readIndex < 0) {
      readIndex += (float)size();
    }
    return lookup(readIndex);
  }
};


} // namespace ky
