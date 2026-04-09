#include "LeftEquivalenceClass.hpp"

#include <algorithm>
#include <sstream>
#include <string>

LeftEquivalenceClass::LeftEquivalenceClass(std::vector<int> itemsetJ, std::vector<int> tidsJ,
                                           std::unordered_map<int, Occurrence> occurrencesJ)
    : itemsetJ(std::move(itemsetJ)),
      tidsJ(std::move(tidsJ)),
      occurrencesJ(std::move(occurrencesJ)) {
    std::sort(this->tidsJ.begin(), this->tidsJ.end());
    this->tidsJ.erase(std::unique(this->tidsJ.begin(), this->tidsJ.end()), this->tidsJ.end());
}

std::string LeftEquivalenceClass::ToString() const {
    std::stringstream ss;

    ss << "EQ:[";
    for (size_t i = 0; i < itemsetJ.size(); i++) {
        ss << itemsetJ[i];
        if (i < itemsetJ.size() - 1) {
            ss << ", ";
        }
    }
    ss << "]";

    return ss.str();
}

bool LeftEquivalenceClass::Equals(LeftEquivalenceClass const& other) const {
    return itemsetJ == other.itemsetJ;
}

bool LeftEquivalenceClass::operator==(LeftEquivalenceClass const& other) const {
    return Equals(other);
}