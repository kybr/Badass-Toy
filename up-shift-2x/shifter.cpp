#include "AudioApp.h"
#include "Gamma/DFT.h"
#include "Gamma/SamplePlayer.h"
using namespace gam;

namespace ky {

template <typename T> T lerp(T a, T b, float t) { return a * (1.0f - t) + b * t; }

template <typename T>
class Array : public std::vector<T> {
  public:
  T lookup(float samples) { 
    size_t to_the_left = static_cast<size_t>(std::floor(samples));
    size_t to_the_right = (to_the_left == (this->size() - 1u)) ? 0u : to_the_left + 1u;
    float t = samples - (float)to_the_left;
    return lerp(this->operator[](to_the_left), this->operator[](to_the_right), t);
  }
  T phasor(float t) { 
    return lookup(this->size() * t);
  }
};

}

class MyApp : public AudioApp {
 public:
  // Short-time Fourier transform
  STFT stft{
      2048,      // Window size
      2048 / 4,  // Hop size; number of samples between transforms
      0,         // Pad size; number of zero-valued samples appended to window
      HANN,      // Window type: BARTLETT, BLACKMAN, BLACKMAN_HARRIS,
                 //		HAMMING, HANN, WELCH, NYQUIST, or RECTANGLE
      COMPLEX    // Format of frequency samples:
                 //		COMPLEX, MAG_PHASE, or MAG_FREQ
  };

  SamplePlayer<> player;
  // ky::Array<STFT::bin_type> data;
  STFT::bin_type data[2048];

  MyApp() {
    player.load("this-is-the-truth.wav");
    //data.resize(2048, 0);
  }


  void onAudio(AudioIOData& io) {
    while (io()) {
      float s = player();

      // const float nyquist = io.fps() / 2.0;
      // const float hertz = float(k) / stft.numBins() * nyquist;

      if (stft(s)) {
        // make a copy of the bins and zero the output bins
        for (int k = 0; k < stft.numBins(); ++k) {
          data[k] = stft.bin(k);
          stft.bin(k) *= 0.0;
        }
        // stretch the spectrum; 2x up-shift
        for (int k = 0; k < stft.numBins() / 2; ++k) {
          stft.bin(k * 2) = data[k];
        }
      }

      s = stft();

      io.out(0) = s;
      io.out(1) = s;
    }
  }
};

int main() { MyApp().start(); }
