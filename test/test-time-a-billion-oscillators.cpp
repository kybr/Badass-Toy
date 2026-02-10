#include "ky.h"

#include <chrono>

template <typename T> void say(const char* prefix, T start, T end) {
  printf("%s: %lf\n", prefix, std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000.0);
}

int main() {
  const int N = 1000000000;

  auto start = std::chrono::high_resolution_clock::now();
  printf("starting...\n");

  ky::Cycle* cycle = new ky::Cycle[N];

  auto allocated = std::chrono::high_resolution_clock::now();
  say("allocated", start, allocated);

  printf("%lu\n", sizeof(cycle));

  auto printed = std::chrono::high_resolution_clock::now();
  say("printed", allocated, printed);

  for (int j = 0; j < N; j++) {
    cycle[j].frequency(1000 + 100 * ky::uniform(), 48000);
  }

  auto initialized = std::chrono::high_resolution_clock::now();
  say("initialized", printed, initialized);

  std::vector<float> buffer;
  for (int i = 0; i < 10; i++) {
    float s = 0;
    for (int j = 0; j < N; j++) {
      s += cycle[j]();
    }
    buffer.push_back(s);
  }

  auto done = std::chrono::high_resolution_clock::now();
  say("done", initialized, done);
  say("total", start, done);


  // results....

  // starting...
  // allocated: 5.383362
  // 8
  // printed: 0.000063
  // initialized: 8.868469
  // done: 302.582090
  // total: 316.833986

  // Karl's M1 pro macbook / Tahoe 26.2
}
