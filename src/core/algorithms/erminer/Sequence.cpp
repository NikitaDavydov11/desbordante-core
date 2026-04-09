#include "Sequence.hpp"

#include <memory>
#include <sstream>

Sequence::Sequence(int id) : id_(id) {}

void Sequence::AddItemset(std::vector<int> const& itemset) {
    itemsets_.push_back(itemset);
}

int Sequence::GetId() const {
    return id_;
}

std::vector<std::vector<int>> const& Sequence::GetItemsets() const {
    return itemsets_;
}

std::vector<int> const& Sequence::Get(int index) const {
    return itemsets_[index];
}

int Sequence::Size() const {
    return static_cast<int>(itemsets_.size());
}

std::string Sequence::ToString() const {
    std::stringstream ss;

    for (auto const& itemset : itemsets_) {
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

std::unique_ptr<Sequence> Sequence::CloneSequenceMinusItems(
        std::unordered_map<int, std::unordered_set<int>> const& mapSequenceID,
        double relativeMinSup) const {
    auto new_sequence = std::make_unique<Sequence>(id_);

    for (auto const& itemset : itemsets_) {
        std::vector<int> new_itemset =
                CloneItemsetMinusItems(itemset, mapSequenceID, relativeMinSup);
        if (!new_itemset.empty()) {
            new_sequence->AddItemset(new_itemset);
        }
    }

    return new_sequence;
}

std::unique_ptr<Sequence> Sequence::CloneSequenceMinusItems(
        double relativeMinSup,
        std::unordered_map<int, std::unordered_set<Sequence*>> const& mapSequenceID) const {
    auto new_sequence = std::make_unique<Sequence>(id_);

    for (auto const& itemset : itemsets_) {
        std::vector<int> new_itemset =
                CloneItemsetMinusItems(relativeMinSup, itemset, mapSequenceID);
        if (!new_itemset.empty()) {
            new_sequence->AddItemset(new_itemset);
        }
    }

    return new_sequence;
}

std::vector<int> Sequence::CloneItemsetMinusItems(
        std::vector<int> const& itemset,
        std::unordered_map<int, std::unordered_set<int>> const& mapSequenceID,
        double minSupportAbsolute) const {
    std::vector<int> new_itemset;

    for (int item : itemset) {
        auto it = mapSequenceID.find(item);
        if (it != mapSequenceID.end() && it->second.size() >= minSupportAbsolute) {
            new_itemset.push_back(item);
        }
    }

    return new_itemset;
}

std::vector<int> Sequence::CloneItemsetMinusItems(
        double relativeMinsup, std::vector<int> const& itemset,
        std::unordered_map<int, std::unordered_set<Sequence*>> const& mapSequenceID) const {
    std::vector<int> new_itemset;

    for (int item : itemset) {
        auto it = mapSequenceID.find(item);
        if (it != mapSequenceID.end() && it->second.size() >= relativeMinsup) {
            new_itemset.push_back(item);
        }
    }

    return new_itemset;
}