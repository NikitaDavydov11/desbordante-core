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
    std::string ToString() const;
};

class AlgoERMiner : public Algorithm {
private:
    long timeStart_;
    long timeEnd_;
    int ruleCount_;

    double minConfidence_;
    int minsuppRelative_;

    std::unique_ptr<SequenceDatabase> database_;

    std::unordered_map<int, std::unordered_map<int, Occurrence>> mapItemCount_;
    std::ofstream writer_;
    ExpandLeftStore store_;
    SparseMatrix matrix_;

    int maxAntecedentSize_;
    int maxConsequentSize_;
    long totalCandidateCount_;
    long candidatePrunedCount_;

    std::vector<Rule> discoveredRules_;

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

    void RunAlgorithm(double minSupport, double minConfidence, std::string const& input,
                      std::string const& output);
    void RunAlgorithm(std::string const& input, std::string const& output, int relativeMinsup,
                      double minConfidence);

    void SetMaxAntecedentSize(int size);
    void SetMaxConsequentSize(int size);

    void PrintStats() const;

    std::vector<Rule> GetRules() const;

private:
    void CalculateFrequencyOfEachItem();
    void GenerateMatrix();

    void CalculateTidsetsIJandJi(std::unordered_map<int, Occurrence> const& occurrencesI,
                                 std::unordered_map<int, Occurrence> const& occurrencesJ,
                                 std::vector<int>& tidsIJ, std::vector<int>& tidsJI) const;

    void RegisterRule11(int intI, int intJ, std::vector<int> const& tidsI,
                        std::vector<int> const& tidsJ, std::vector<int> const& tidsIJ,
                        std::unordered_map<int, Occurrence> const& occurrencesI,
                        std::unordered_map<int, Occurrence> const& occurrencesJ,
                        std::unordered_map<int, LeftEquivalenceClass>& mapEclassLeft,
                        std::unordered_map<int, RightEquivalenceClass>& mapEclassRight);

    void ExpandLeft(LeftEquivalenceClass& eclass);
    void ExpandRight(RightEquivalenceClass& eclass);
    void SaveRule(std::vector<int> const& tidsIJ, double confIJ, std::vector<int> const& itemsetI,
                  std::vector<int> const& itemsetJ);

    std::vector<int> Concatenate(std::vector<int> const& itemset, int item) const;
};

}  // namespace algos