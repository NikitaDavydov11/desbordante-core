#pragma once
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ExpandLeftStore.hpp"
#include "LeftEquivalenceClass.hpp"
#include "LeftRule.hpp"
#include "Occurence.hpp"
#include "RightEquivalenceClass.hpp"
#include "RightRule.hpp"
#include "SequenceDatabase.hpp"
#include "SparseMatrix.hpp"
#include "core/algorithms/algorithm.h"
#include "core/config/names.h"
#include "core/config/option.h"

namespace algos {

struct Rule {
    std::vector<int> antecedent;
    std::vector<int> consequent;
    int support;
    double confidence;
    std::string toString() const;
};

class AlgoERMiner : public Algorithm {
private:
    long timeStart;
    long timeEnd;
    int ruleCount;

    double minConfidence;
    int minsuppRelative;

    std::unique_ptr<SequenceDatabase> database;

    std::unordered_map<int, std::unordered_map<int, Occurence>> mapItemCount;
    std::ofstream writer;
    ExpandLeftStore store;
    SparseMatrix matrix;

    int maxAntecedentSize;
    int maxConsequentSize;
    long totalCandidateCount;
    long candidatePrunedCount;

    std::vector<Rule> discoveredRules;

    std::string input_path_;
    std::string output_path_;
    double min_support_;
    double min_confidence_;

public:
    AlgoERMiner();

    void RegisterOptions();

    void ResetState() override;
    void LoadDataInternal() override;
    unsigned long long ExecuteInternal() override;

    void runAlgorithm(double minSupport, double minConfidence, std::string const& input,
                      std::string const& output);
    void runAlgorithm(std::string const& input, std::string const& output, int relativeMinsup,
                      double minConfidence);

    void setMaxAntecedentSize(int size);
    void setMaxConsequentSize(int size);

    void printStats() const;

    std::vector<Rule> getRules() const;

private:
    void calculateFrequencyOfEachItem();
    void generateMatrix();

    void calculateTidsetsIJandJI(std::unordered_map<int, Occurence> const& occurencesI,
                                 std::unordered_map<int, Occurence> const& occurencesJ,
                                 std::vector<int>& tidsIJ, std::vector<int>& tidsJI) const;

    void registerRule11(int intI, int intJ, std::vector<int> const& tidsI,
                        std::vector<int> const& tidsJ, std::vector<int> const& tidsIJ,
                        std::unordered_map<int, Occurence> const& occurencesI,
                        std::unordered_map<int, Occurence> const& occurencesJ,
                        std::unordered_map<int, LeftEquivalenceClass>& mapEclassLeft,
                        std::unordered_map<int, RightEquivalenceClass>& mapEclassRight);

    void expandLeft(LeftEquivalenceClass& eclass);
    void expandRight(RightEquivalenceClass& eclass);
    void saveRule(std::vector<int> const& tidsIJ, double confIJ, std::vector<int> const& itemsetI,
                  std::vector<int> const& itemsetJ);

    std::vector<int> concatenate(std::vector<int> const& itemset, int item) const;
};

}  // namespace algos