#include "SparseMatrix.hpp"

void SparseMatrix::increaseCountOfPair(int i, int j) {
    if (i < j) {
        return;
    }

    matrix[i][j]++;
}

int SparseMatrix::getCount(int i, int j) const {
    auto outerIt = matrix.find(i);
    if (outerIt == matrix.end()) {
        return 0;
    }

    auto const& innerMap = outerIt->second;
    auto innerIt = innerMap.find(j);
    if (innerIt == innerMap.end()) {
        return 0;
    }

    return innerIt->second;
}

std::unordered_map<int, std::unordered_map<int, int>> const& SparseMatrix::getMatrix() const {
    return matrix;
}