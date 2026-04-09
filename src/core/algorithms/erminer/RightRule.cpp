#include "RightRule.hpp"

#include <algorithm>
#include <sstream>
#include <string>

RightRule::RightRule(std::vector<int> itemsetJ, std::vector<int> tidsJ, std::vector<int> tidsIJ,
                     std::unordered_map<int, Occurrence> occurrencesJ)
    : itemsetJ(std::move(itemsetJ)),
      tidsJ(std::move(tidsJ)),
      tidsIJ(std::move(tidsIJ)),
      occurrencesJ(std::move(occurrencesJ)) {
    std::sort(this->tidsJ.begin(), this->tidsJ.end());
    this->tidsJ.erase(std::unique(this->tidsJ.begin(), this->tidsJ.end()), this->tidsJ.end());

    std::sort(this->tidsIJ.begin(), this->tidsIJ.end());
    this->tidsIJ.erase(std::unique(this->tidsIJ.begin(), this->tidsIJ.end()), this->tidsIJ.end());
}

std::string RightRule::ToString() const {
    std::stringstream ss;

    ss << " ==> [";
    for (size_t i = 0; i < itemsetJ.size(); i++) {
        ss << itemsetJ[i];
        if (i < itemsetJ.size() - 1) {
            ss << ", ";
        }
    }
    ss << "]";

    return ss.str();
}