#include "../ky.h"

int main() {
  ky::SlewRateLimit filter;
  filter.configure(-1.0, 0.8, 200.0);
  for (int i = 0; i < 10; i++) {
    printf("%lf\n", filter(0));
  }
}
