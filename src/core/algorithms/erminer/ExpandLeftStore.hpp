#pragma once
#include <list>
#include <unordered_map>
#include <vector>

#include "LeftEquivalenceClass.hpp"
#include "LeftRule.hpp"
#include "Occurence.hpp"

class ExpandLeftStore {
public:
    std::unordered_map<int, std::unordered_map<int, std::list<LeftEquivalenceClass>>> store;

    ExpandLeftStore() = default;

    void registerRule(LeftRule const& leftRule, std::vector<int> const& itemsetJ,
                      std::vector<int> const& tidsJ,
                      std::unordered_map<int, Occurence> const& occurencesJ);

private:
    int computeHash(std::vector<int> const& vec) const;
};