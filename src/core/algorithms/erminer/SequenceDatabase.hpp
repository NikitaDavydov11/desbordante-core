#pragma once
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "Sequence.hpp"

class SequenceDatabase {
private:
    std::vector<std::unique_ptr<Sequence>> sequences_;

public:
    SequenceDatabase() = default;

    void LoadFile(std::string const& path);

    void AddSequence(std::unique_ptr<Sequence> sequence);

    int Size() const;

    std::vector<std::unique_ptr<Sequence>> const& GetSequences() const;

    std::unordered_set<int> GetSequenceIDs() const;

    std::string ToString() const;

    void PrintDatabaseStats() const;

private:
    void AddSequence(std::vector<std::string> const& tokens);
};