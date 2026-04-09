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

    int getAbsoluteSupport() const;

    std::vector<int> getItems() const;

    int get(int index) const;

    void setTIDs(std::vector<int> const& listTransactionIds);

    int size() const;

    std::vector<int> getTransactionsIds() const;

    Itemset cloneItemSetMinusAnItemset(Itemset const& itemsetToNotKeep) const;
    Itemset cloneItemSetMinusOneItem(int itemToRemove) const;
};