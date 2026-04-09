#include "Itemset.hpp"

#include <algorithm>

Itemset::Itemset() = default;

Itemset::Itemset(int item) {
    itemset.push_back(item);
}

Itemset::Itemset(std::vector<int> items) : itemset(std::move(items)) {
    std::sort(itemset.begin(), itemset.end());
}

int Itemset::GetAbsoluteSupport() const {
    return static_cast<int>(transactionsIds.size());
}

std::vector<int> Itemset::GetItems() const {
    return itemset;
}

int Itemset::Get(int index) const {
    return itemset[index];
}

void Itemset::SetTiDs(std::vector<int> const& listTransactionIds) {
    transactionsIds = listTransactionIds;
    std::sort(transactionsIds.begin(), transactionsIds.end());
    transactionsIds.erase(std::unique(transactionsIds.begin(), transactionsIds.end()),
                          transactionsIds.end());
}

int Itemset::Size() const {
    return static_cast<int>(itemset.size());
}

std::vector<int> Itemset::GetTransactionsIds() const {
    return transactionsIds;
}

Itemset Itemset::CloneItemSetMinusAnItemset(Itemset const& itemsetToNotKeep) const {
    std::vector<int> new_itemset;

    for (int item : itemset) {
        bool found = false;
        for (int exclude_item : itemsetToNotKeep.itemset) {
            if (item == exclude_item) {
                found = true;
                break;
            }
        }

        if (!found) {
            new_itemset.push_back(item);
        }
    }

    return Itemset(new_itemset);
}

Itemset Itemset::CloneItemSetMinusOneItem(int itemToRemove) const {
    std::vector<int> new_itemset;

    for (int item : itemset) {
        if (item != itemToRemove) {
            new_itemset.push_back(item);
        }
    }

    return Itemset(new_itemset);
}