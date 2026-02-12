// This was LLM-generated from models stolen from the free software movement.
// Thanks.

#pragma once

#include <stdexcept>
#include <vector>

template <typename T>
using Matrix = std::vector<std::vector<T>>;

template <typename T>
Matrix<T> multiply(const Matrix<T>& A, const Matrix<T>& B) {
  if (A.empty() || B.empty() || A[0].empty() || B[0].empty()) {
    throw std::invalid_argument("Matrices cannot be empty");
  }

  int rowsA = A.size();
  int colsA = A[0].size();
  int rowsB = B.size();
  int colsB = B[0].size();

  // Matrix multiplication condition: Cols A must equal Rows B
  if (colsA != rowsB) {
    throw std::invalid_argument("Incompatible dimensions for multiplication");
  }

  // Initialize result matrix with zeros
  Matrix<T> C(rowsA, std::vector<T>(colsB, 0));

  // Perform multiplication
  for (int i = 0; i < rowsA; ++i) {
    for (int j = 0; j < colsB; ++j) {
      for (int k = 0; k < colsA; ++k) {
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }
  return C;
}