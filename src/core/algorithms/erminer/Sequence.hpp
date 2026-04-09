#pragma once
#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Sequence;

class Sequence {
private:
    std::vector<std::vector<int>> itemsets_;
    int id_;

public:
    explicit Sequence(int id_);

    void AddItemset(std::vector<int> const& itemset);

    int GetId() const;

    std::vector<std::vector<int>> const& GetItemsets() const;
    std::vector<int> const& Get(int index) const;

    int Size() const;

    std::string ToString() const;

    std::unique_ptr<Sequence> CloneSequenceMinusItems(
            std::unordered_map<int, std::unordered_set<int>> const& mapSequenceID,
            double relativeMinSup) const;

    std::unique_ptr<Sequence> CloneSequenceMinusItems(
            double relativeMinSup,
            std::unordered_map<int, std::unordered_set<Sequence*>> const& mapSequenceID) const;

private:
    std::vector<int> CloneItemsetMinusItems(
            std::vector<int> const& itemset,
            std::unordered_map<int, std::unordered_set<int>> const& mapSequenceID,
            double minSupportAbsolute) const;

    std::vector<int> CloneItemsetMinusItems(
            double relativeMinsup, std::vector<int> const& itemset,
            std::unordered_map<int, std::unordered_set<Sequence*>> const& mapSequenceID) const;
};