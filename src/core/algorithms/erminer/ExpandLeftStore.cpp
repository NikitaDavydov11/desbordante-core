#include "ExpandLeftStore.hpp"

#include <algorithm>
#include <functional>

void ExpandLeftStore::RegisterRule(LeftRule const& leftRule, std::vector<int> const& itemsetJ,
                                   std::vector<int> const& tidsJ,
                                   std::unordered_map<int, Occurrence> const& occurrencesJ) {
    int size = static_cast<int>(itemsetJ.size());
    int hash = ComputeHash(itemsetJ);

    auto& size_map = store[size];
    auto& class_list = size_map[hash];

    for (auto& eclass : class_list) {
        if (eclass.itemsetJ == itemsetJ) {
            eclass.rules.push_back(leftRule);
            return;
        }
    }

    std::vector<int> sorted_tids_j = tidsJ;
    std::sort(sorted_tids_j.begin(), sorted_tids_j.end());
    sorted_tids_j.erase(std::unique(sorted_tids_j.begin(), sorted_tids_j.end()),
                        sorted_tids_j.end());

    LeftEquivalenceClass new_class(itemsetJ, sorted_tids_j, occurrencesJ);
    new_class.rules.push_back(leftRule);
    class_list.push_back(std::move(new_class));
}

int ExpandLeftStore::ComputeHash(std::vector<int> const& vec) const {
    int hash = 1;
    for (int item : vec) {
        hash = 31 * hash + item;
    }
    return hash;
}