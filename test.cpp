#include <iostream>
#include <random>
#include <vector>

#include "ky.h"
#include "matrix.h"

float normal() {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<float> d;
    return d(gen);
}
class Reverb {
  Matrix<float> y;
  Matrix<float> Q;
  std::vector<float> M, g;
  // 0.01388, 0.00452, 0.00148 .. allpass
  std::vector<ky::DelayLine> delay;
  public:

  Reverb() {
    y.resize(5);
    for (int i = 0; i < y.size(); i++) {
      std::vector<float> v;
      Q.emplace_back(v);
      for (int j = 0; j < y.size(); j++) {
        Q.back().push_back(normal() * 0.1);
      }
    }

    delay.resize(y.size());
    for (int i = 0; i < y.size(); i++) {
      delay[i].resize(48000);
    }
  }

  float operator()(float f) {
    // 1) read the delay lines...
    //
    float output = 0;
    for (int i = 0; i < y.size(); i++) {
      y[0][i] = delay[i].read(M[i]);
      output += y[0][i];
    }

    // 2) matrix multiply: Q * y -> result
    //
    auto result = multiply(Q, y);

    // 3) scale result by g
    //
    for (int i = 0; i < y.size(); i++) {
      result[0][i] *= g[i];
    }

    // 4) add f to result
    //
    for (int i = 0; i < y.size(); i++) {
      result[0][i] += f;
    }

    // 5) write result to delay lines
    for (int i = 0; i < y.size(); i++) {
      delay[i].write(result[0][i]);
    }

    // 6) return mixdown of y + a little f (dry)
    //
    return output + 0.1 * f;
  }
};


int main() {
  // 2x3 Matrix
  Matrix<int> A = {{1, 2, 3}, {4, 5, 6}};

  // 3x2 Matrix
  Matrix<int> B = {{7, 8}, {9, 1}, {2, 3}};

  try {
    Matrix<int> result = multiply(A, B);
    // Result is 2x2:
    // [ (1*7+2*9+3*2) (1*8+2*1+3*3) ] = [ 31 19 ]
    // [ (4*7+5*9+6*2) (4*8+5*1+6*3) ] = [ 85 57 ]

    for (const auto& row : result) {
      for (int val : row) std::cout << val << " ";
      std::cout << "\n";
    }
  } catch (const std::invalid_argument& e) {
    std::cerr << "Error: " << e.what() << "\n";
  }
  
  // C order is row-major
  // GLSL is column-major

  // row major (?)
  Matrix<float> Q = {
    {0.1, 0.8, 0.2},
    {0.2, 0.2, 0.6},
    {0.7, 0.0, 0.2},
  };

  Matrix<float> v = {
    {1.0},
    {0.0},
    {0.0},
  };

  Matrix<float> result = multiply(Q, v);
  for (const auto& row : result) {
    for (auto val : row) std::cout << val << " ";
    std::cout << "\n";
  }

  printf("%f\n", normal());
  printf("%f\n", normal());
  printf("%f\n", normal());
  printf("%f\n", normal());
  printf("%f\n", normal());

  Reverb reverb;
  reverb(0);

  return 0;
}
