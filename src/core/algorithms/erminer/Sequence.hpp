#pragma once
#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Sequence;

class Sequence {
private:
    std::vector<std::vector<int>> itemsets;
    int id;

public:
    explicit Sequence(int id);

    void addItemset(std::vector<int> const& itemset);

    int getId() const;

    std::vector<std::vector<int>> const& getItemsets() const;
    std::vector<int> const& get(int index) const;

    int size() const;

    std::string toString() const;

    std::unique_ptr<Sequence> cloneSequenceMinusItems(
            std::unordered_map<int, std::unordered_set<int>> const& mapSequenceID,
            double relativeMinSup) const;

    std::unique_ptr<Sequence> cloneSequenceMinusItems(
            double relativeMinSup,
            std::unordered_map<int, std::unordered_set<Sequence*>> const& mapSequenceID) const;

private:
    std::vector<int> cloneItemsetMinusItems(
            std::vector<int> const& itemset,
            std::unordered_map<int, std::unordered_set<int>> const& mapSequenceID,
            double minSupportAbsolute) const;

    std::vector<int> cloneItemsetMinusItems(
            double relativeMinsup, std::vector<int> const& itemset,
            std::unordered_map<int, std::unordered_set<Sequence*>> const& mapSequenceID) const;
};