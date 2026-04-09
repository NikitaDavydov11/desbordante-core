#include "SparseMatrix.hpp"

void SparseMatrix::IncreaseCountOfPair(int i, int j) {
    if (i < j) {
        return;
    }

    matrix[i][j]++;
}

int SparseMatrix::GetCount(int i, int j) const {
    auto outer_it = matrix.find(i);
    if (outer_it == matrix.end()) {
        return 0;
    }

    auto const& inner_map = outer_it->second;
    auto inner_it = inner_map.find(j);
    if (inner_it == inner_map.end()) {
        return 0;
    }

    return inner_it->second;
}

std::unordered_map<int, std::unordered_map<int, int>> const& SparseMatrix::GetMatrix() const {
    return matrix;
}