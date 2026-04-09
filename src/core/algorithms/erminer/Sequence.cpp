#include "Sequence.hpp"

#include <memory>
#include <sstream>

Sequence::Sequence(int id) : id(id) {}

void Sequence::addItemset(std::vector<int> const& itemset) {
    itemsets.push_back(itemset);
}

int Sequence::getId() const {
    return id;
}

std::vector<std::vector<int>> const& Sequence::getItemsets() const {
    return itemsets;
}

std::vector<int> const& Sequence::get(int index) const {
    return itemsets[index];
}

int Sequence::size() const {
    return static_cast<int>(itemsets.size());
}

std::string Sequence::toString() const {
    std::stringstream ss;

    for (auto const& itemset : itemsets) {
        ss << '(';
        for (size_t i = 0; i < itemset.size(); i++) {
            ss << itemset[i];
            if (i < itemset.size() - 1) {
                ss << ' ';
            }
        }
        ss << ')';
    }
    ss << "    ";

    return ss.str();
}

std::unique_ptr<Sequence> Sequence::cloneSequenceMinusItems(
        std::unordered_map<int, std::unordered_set<int>> const& mapSequenceID,
        double relativeMinSup) const {
    auto newSequence = std::make_unique<Sequence>(id);

    for (auto const& itemset : itemsets) {
        std::vector<int> newItemset =
                cloneItemsetMinusItems(itemset, mapSequenceID, relativeMinSup);
        if (!newItemset.empty()) {
            newSequence->addItemset(newItemset);
        }
    }

    return newSequence;
}

std::unique_ptr<Sequence> Sequence::cloneSequenceMinusItems(
        double relativeMinSup,
        std::unordered_map<int, std::unordered_set<Sequence*>> const& mapSequenceID) const {
    auto newSequence = std::make_unique<Sequence>(id);

    for (auto const& itemset : itemsets) {
        std::vector<int> newItemset =
                cloneItemsetMinusItems(relativeMinSup, itemset, mapSequenceID);
        if (!newItemset.empty()) {
            newSequence->addItemset(newItemset);
        }
    }

    return newSequence;
}

std::vector<int> Sequence::cloneItemsetMinusItems(
        std::vector<int> const& itemset,
        std::unordered_map<int, std::unordered_set<int>> const& mapSequenceID,
        double minSupportAbsolute) const {
    std::vector<int> newItemset;

    for (int item : itemset) {
        auto it = mapSequenceID.find(item);
        if (it != mapSequenceID.end() && it->second.size() >= minSupportAbsolute) {
            newItemset.push_back(item);
        }
    }

    return newItemset;
}

std::vector<int> Sequence::cloneItemsetMinusItems(
        double relativeMinsup, std::vector<int> const& itemset,
        std::unordered_map<int, std::unordered_set<Sequence*>> const& mapSequenceID) const {
    std::vector<int> newItemset;

    for (int item : itemset) {
        auto it = mapSequenceID.find(item);
        if (it != mapSequenceID.end() && it->second.size() >= relativeMinsup) {
            newItemset.push_back(item);
        }
    }

    return newItemset;
}