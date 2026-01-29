#include "ky.h"

int main() {
  // ky::Cycle c;
  // c.frequency(440, 48000);
  // for (int i = 0; i < 100000; i++) {
  //     c();
  //   //printf("%lf\n", c() * 0.5);
  // }

  for (int i = 0; i < 100000; i++) {
    printf("%lf\n", ky::sint((float)i / 100000));
    //printf("%lf\n", sinf((2.0f * M_PI * i) / 100000));
  }
}
