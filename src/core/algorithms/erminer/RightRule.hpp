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
    std::unordered_map<int, Occurence> occurencesJ;

    RightRule(std::vector<int> itemsetJ, std::vector<int> tidsJ, std::vector<int> tidsIJ,
              std::unordered_map<int, Occurence> occurencesJ);
    std::string toString() const;
};