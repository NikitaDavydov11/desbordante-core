#pragma once
#include <algorithm>
#include <vector>

class Itemset {
public:
    std::vector<int> itemset;
    std::vector<int> transactionsIds;

    Itemset();
    Itemset(int item);
    Itemset(std::vector<int> items);

    int GetAbsoluteSupport() const;

    std::vector<int> GetItems() const;

    int Get(int index) const;

    void SetTiDs(std::vector<int> const& listTransactionIds);

    int Size() const;

    std::vector<int> GetTransactionsIds() const;

    Itemset CloneItemSetMinusAnItemset(Itemset const& itemsetToNotKeep) const;
    Itemset CloneItemSetMinusOneItem(int itemToRemove) const;
};