#pragma once
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "Sequence.hpp"

class SequenceDatabase {
private:
    std::vector<std::unique_ptr<Sequence>> sequences;

public:
    SequenceDatabase() = default;

    void loadFile(std::string const& path);

    void addSequence(std::unique_ptr<Sequence> sequence);

    int size() const;

    std::vector<std::unique_ptr<Sequence>> const& getSequences() const;

    std::unordered_set<int> getSequenceIDs() const;

    std::string toString() const;

    void printDatabaseStats() const;

private:
    void addSequence(std::vector<std::string> const& tokens);
};