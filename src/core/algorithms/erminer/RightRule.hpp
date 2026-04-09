#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "Occurence.hpp"

class RightRule {
public:
    std::vector<int> itemsetJ;
    std::vector<int> tidsJ;
    std::vector<int> tidsIJ;
    std::unordered_map<int, Occurrence> occurrencesJ;

    RightRule(std::vector<int> itemsetJ, std::vector<int> tidsJ, std::vector<int> tidsIJ,
              std::unordered_map<int, Occurrence> occurrencesJ);
    std::string ToString() const;
};