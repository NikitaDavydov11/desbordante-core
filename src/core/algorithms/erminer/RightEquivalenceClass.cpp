#include "RightEquivalenceClass.hpp"

#include <algorithm>
#include <sstream>
#include <string>

RightEquivalenceClass::RightEquivalenceClass(std::vector<int> itemsetI, std::vector<int> tidsI,
                                             std::unordered_map<int, Occurrence> occurrencesI)
    : itemsetI(std::move(itemsetI)),
      tidsI(std::move(tidsI)),
      occurrencesI(std::move(occurrencesI)) {
    std::sort(this->tidsI.begin(), this->tidsI.end());
    this->tidsI.erase(std::unique(this->tidsI.begin(), this->tidsI.end()), this->tidsI.end());
}

std::string RightEquivalenceClass::ToString() const {
    std::stringstream ss;

    ss << "[";
    for (size_t i = 0; i < itemsetI.size(); i++) {
        ss << itemsetI[i];
        if (i < itemsetI.size() - 1) {
            ss << ", ";
        }
    }
    ss << "] ==> EQ";

    return ss.str();
}

bool RightEquivalenceClass::Equals(RightEquivalenceClass const& other) const {
    return itemsetI == other.itemsetI;
}

bool RightEquivalenceClass::operator==(RightEquivalenceClass const& other) const {
    return Equals(other);
}