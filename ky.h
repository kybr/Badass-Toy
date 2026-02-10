#pragma once

#include <cmath>
#include <vector>
#include <numbers>

namespace ky {

///////////////////////////////////////////////////////////////////////////////
//// Constants ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

constexpr float tau = std::numbers::pi_v<float> * 2.0f;

///////////////////////////////////////////////////////////////////////////////
//// Functions ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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

// XXX we will improve this later...
static float uniform() {
  // from the FAUST:
  // rand  = +(12345)~*(1103515245);
  // w   = rand/2147483647.0;
  static int history = 0;
  history *= 1103515245;
  history += 12345;
  return history / 2147483647.0;
}

///////////////////////////////////////////////////////////////////////////////
//// Support Classes //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// 1-sample delay
class History {
  float value = 0;

  public:
  float operator()() { return value; }
  float operator()(float v) {
    float tmp = value;
    value = v;
    return tmp;
  }
};

// Takes the derivative of a signal
// the backward difference
class Delta {
  History history;

  public:
  float operator()(float f) {
    return f - history(f);
  }
};
class ArrayFloat : public std::vector<float> {
  public:
  float lookup(float samples) { 
    size_t to_the_left = static_cast<size_t>(std::floor(samples));
    size_t to_the_right = (to_the_left == (size() - 1u)) ? 0u : to_the_left + 1u;
    float t = samples - (float)to_the_left;
    return lerp(operator[](to_the_left), operator[](to_the_right), t);
  }
  float phasor(float t) { 
    return lookup(size() * t);
  }
};

///////////////////////////////////////////////////////////////////////////////
//// Sines ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/// (0, 1)
inline float sin7(float x) {
    // 7 multiplies + 7 addition/subtraction
    // 14 operations
    return x * (x * (x * (x * (x * (x * (66.5723768716453f * x - 233.003319050759f) + 275.754490892928f) - 106.877929605423f) + 0.156842000875713f) - 9.85899292126983f) + 7.25653181200263f) - 8.88178419700125e-16f;
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

///////////////////////////////////////////////////////////////////////////////
//// Oscillators //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// functor class
class Phasor {
  float increment = 0; // normalized frequency
  float value = 0;

 public:

  float operator()() {
    float v = value; // remember value
    value += increment; // "side effect"
    if (value >= 1.0f) {
      value -= 1.0f; // wrap phase
    }
    return v;
  }

  void frequency(float hertz, float sampleRate) {
    increment = hertz / sampleRate;
  }
};

// Fires at a given frequency...
class Timer {
  float increment = 0;
  float value = 0;

  public:
  bool operator()() {
    if (value >= 1.0f) {
      value -= 1.0f;
      return true;
    }
    value += increment;
    return false;
  }

  void frequency(float hertz, float sampleRate) {
    increment = hertz / sampleRate;
  }
};

// Quasi-Bandlimited Frequency Modulation

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
    osc = (osc + std::sin(tau * (phase + osc * scaling * t))) * 0.5f;

    // compensate HF rolloff
    float out = a0 * osc + a1 * in_hist;
    in_hist = osc;
    out = out + DC;  // compensate DC offset

    return out * norm;
  }
};

struct Cycle : public Phasor {
  float operator()() {
    return sint(Phasor::operator()());
  }
};

///////////////////////////////////////////////////////////////////////////////
//// Delay ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
//// Filters //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Average this input sample with the last input sample
class TwoSampleMean {
  History history;

  public:
  float operator()(float f) {
    return (f + history(f)) / 2.0f;
  }
};

// Another simple low-pass filter
class OnePole {
  float b0 = 1, a1 = 0, yn1 = 0;

  public:
  void frequency(float hertz, float samplerate) {
    a1 = exp(-tau * hertz / samplerate);
    b0 = 1.0f - a1;
  }
  float operator()(float xn) { return yn1 = b0 * xn + a1 * yn1; }
};

// Also a low-pass filter, but non-linear
class SlewRateLimit {
  float limit = 0;
  float value = 0;
  public:
  void configure(float v, float r, float samplerate) {
    value = v;
    limit = r / samplerate;
  }
  void slewrate(float r, float samplerate) {
    limit = r / samplerate;
  }
  float operator()(float f) {
    float v = value;

    // side effect....
    float delta = f - value;
    if (delta > limit) {
      delta = limit;
    }
    else if (delta < -limit) {
      delta = -limit;
    }
    value += delta;

    return v;
  }
};


///////////////////////////////////////////////////////////////////////////////
//// Synths ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class PluckedString : public DelayLine {
  TwoSampleMean filter;

  float gain = 1;
  float t60 = 1;
  float delayTime = 1;  // in seconds

  public:

  void frequency(float hertz) { period(1 / hertz); }
  void period(float seconds) {
    delayTime = seconds;
    recalculate();
  }

  void decayTime(float _t60) {
    t60 = _t60;
    recalculate();
  }

  void set(float frequency, float decayTime) {
    delayTime = 1 / frequency;
    t60 = decayTime;
    recalculate();
  }

  // given t60 and frequency (seconds and Hertz), calculate the gain...
  //
  // for a given frequency, our algorithm applies *gain* frequency-many
  // times per second. given a t60 time we can calculate how many times (n)
  // gain will be applied in those t60 seconds. we want to reduce the signal
  // by 60dB over t60 seconds or over n-many applications. this means that
  // we want gain to be a number that, when multiplied by itself n times,
  // becomes 60 dB quieter than it began.
  //
  void recalculate() {
    int n = t60 / delayTime;
    gain = pow(dbtoa(-60), 1.0f / n);
    //printf("t:%f\tf:%f\tn:%d\tg:%f\n", t60, 1 / delayTime, n, gain);
    //fflush(stdout);
  }

  float operator()() {
    float v = filter(read(48000 * delayTime)) * gain;
    write(v);
    return v;
  }

  void pluck(float gain = 1) {
    // put noise in the last N sample memory positions. N depends on
    // frequency
    //
    int n = int(ceil(delayTime * 48000));
    for (int i = 0; i < n; ++i) {
      write(uniform() * gain);
    }
  }
};

} // namespace ky
