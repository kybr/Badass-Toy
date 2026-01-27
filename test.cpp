#include "ky.h"

int main() {
    ky::TableSine s;
  s.frequency(440, 48000);
  for (int i = 0; i < 100000; i++) {
    printf("%lf\n", s() * 0.5);
  }
}
