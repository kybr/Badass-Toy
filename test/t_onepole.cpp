#include "../ky.h"

int main() {
  ky::OnePole filter;
  filter.frequency(10, 48000);
  for (int i = 0; i < 50000; i++) {
    printf("%lf\n", filter(1));
  }
  for (int i = 0; i < 50000; i++) {
    printf("%lf\n", filter(0));
  }
}
