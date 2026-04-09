#pragma once
#include <list>
#include <unordered_map>
#include <vector>

#include "LeftRule.hpp"
#include "Occurence.hpp"

class LeftEquivalenceClass {
public:
    std::vector<int> itemsetJ;
    std::vector<int> tidsJ;
    std::unordered_map<int, Occurence> occurencesJ;
    std::list<LeftRule> rules;

    LeftEquivalenceClass(std::vector<int> itemsetJ, std::vector<int> tidsJ,
                         std::unordered_map<int, Occurence> occurencesJ);

    std::string toString() const;
    bool equals(LeftEquivalenceClass const& other) const;
    bool operator==(LeftEquivalenceClass const& other) const;
};