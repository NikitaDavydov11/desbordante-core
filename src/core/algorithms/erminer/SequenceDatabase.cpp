#include "SequenceDatabase.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

void SequenceDatabase::LoadFile(std::string const& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == '%' || line[0] == '@') {
            continue;
        }

        std::vector<std::string> tokens;
        std::istringstream iss(line);
        std::string token;

        while (iss >> token) {
            tokens.push_back(token);
        }

        if (!tokens.empty()) {
            AddSequence(tokens);
        }
    }
}

void SequenceDatabase::AddSequence(std::vector<std::string> const& tokens) {
    auto sequence = std::make_unique<Sequence>(static_cast<int>(sequences_.size()));
    std::vector<int> itemset;

    for (auto const& token : tokens) {
        if (!token.empty() && token[0] == '<') {
            continue;
        } else if (token == "-1") {
            if (!itemset.empty()) {
                sequence->AddItemset(itemset);
                itemset.clear();
            }
        } else if (token == "-2") {
            if (!itemset.empty()) {
                sequence->AddItemset(itemset);
            }
            sequences_.push_back(std::move(sequence));
            return;
        } else {
            try {
                itemset.push_back(std::stoi(token));
            } catch (std::invalid_argument const& e) {
                throw std::runtime_error("Invalid item in sequence: " + token);
            }
        }
    }

    if (!itemset.empty()) {
        sequence->AddItemset(itemset);
    }
    sequences_.push_back(std::move(sequence));
}

void SequenceDatabase::AddSequence(std::unique_ptr<Sequence> sequence) {
    sequences_.push_back(std::move(sequence));
}

int SequenceDatabase::Size() const {
    return static_cast<int>(sequences_.size());
}

std::vector<std::unique_ptr<Sequence>> const& SequenceDatabase::GetSequences() const {
    return sequences_;
}

std::unordered_set<int> SequenceDatabase::GetSequenceIDs() const {
    std::unordered_set<int> ids;
    for (auto const& seq : sequences_) {
        ids.insert(seq->GetId());
    }
    return ids;
}

std::string SequenceDatabase::ToString() const {
    std::stringstream ss;
    for (auto const& sequence : sequences_) {
        ss << sequence->GetId() << ":  " << sequence->ToString() << "\n";
    }
    return ss.str();
}

void SequenceDatabase::PrintDatabaseStats() const {
    std::cout << "============  STATS ==========\n";
    std::cout << "Number of sequences : " << sequences_.size() << "\n";

    if (sequences_.empty()) {
        std::cout << "mean size: 0\n";
        return;
    }

    long total_size = 0;
    for (auto const& seq : sequences_) {
        total_size += seq->Size();
    }

    double mean_size = static_cast<double>(total_size) / sequences_.size();
    std::cout << "mean size: " << mean_size << "\n";
}