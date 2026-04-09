#pragma once
#include <unordered_map>

class SparseMatrix {
public:
    std::unordered_map<int, std::unordered_map<int, int>> matrix;

    SparseMatrix() = default;

    void IncreaseCountOfPair(int i, int j);

    int GetCount(int i, int j) const;

    std::unordered_map<int, std::unordered_map<int, int>> const& GetMatrix() const;
};