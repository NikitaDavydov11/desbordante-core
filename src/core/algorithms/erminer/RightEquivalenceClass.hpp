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
    std::unordered_map<int, Occurence> occurencesI;
    std::list<RightRule> rules;

    RightEquivalenceClass(std::vector<int> itemsetI, std::vector<int> tidsI,
                          std::unordered_map<int, Occurence> occurencesI);

    std::string toString() const;

    bool equals(RightEquivalenceClass const& other) const;

    bool operator==(RightEquivalenceClass const& other) const;
};