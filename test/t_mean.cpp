#include "../ky.h"

int main() {
  ky::TwoSampleMean filter;
  for (int i = 0; i < 5; i++) {
    printf("%lf\n", filter(1));
  }
  for (int i = 0; i < 10; i++) {
    printf("%lf\n", filter(0));
  }
}
