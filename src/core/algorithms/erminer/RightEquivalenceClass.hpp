#pragma once
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include "Occurence.hpp"
#include "RightRule.hpp"

class RightEquivalenceClass {
public:
    std::vector<int> itemsetI;
    std::vector<int> tidsI;
    std::unordered_map<int, Occurrence> occurrencesI;
    std::list<RightRule> rules;

    RightEquivalenceClass(std::vector<int> itemsetI, std::vector<int> tidsI,
                          std::unordered_map<int, Occurrence> occurrencesI);

    std::string ToString() const;

    bool Equals(RightEquivalenceClass const& other) const;

    bool operator==(RightEquivalenceClass const& other) const;
};