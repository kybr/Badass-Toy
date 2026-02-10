#include "../ky.h"

int main() {
  ky::PluckedString string;
  string.resize(48000, 0);
  string.set(300, 0.7);
  string.pluck();

  for (int i = 0; i < 100; i++) {
    printf("%lf\n", string());
  }
}
