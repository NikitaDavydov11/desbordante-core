#include "ERMiner.hpp"

#include <algorithm>
#include <cfloat>
#include <chrono>
#include <cmath>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

#define DESBORDANTE_OPTION_USING             \
    using config::names::kMinimumSupport;    \
    using config::names::kMinimumConfidence; \
    using config::names::kTable;

namespace algos {

AlgoERMiner::AlgoERMiner()
    : Algorithm({}),
      timeStart_(0),
      timeEnd_(0),
      ruleCount_(0),
      minConfidence_(0),
      minsuppRelative_(0),
      database_(std::make_unique<SequenceDatabase>()),
      maxAntecedentSize_(INT_MAX),
      maxConsequentSize_(INT_MAX),
      totalCandidateCount_(0),
      candidatePrunedCount_(0),
      min_support_(0),
      min_confidence_(0) {
    RegisterOptions();
    MakeOptionsAvailable({config::names::kTable, config::names::kMinimumSupport,
                          config::names::kMinimumConfidence});
}

void AlgoERMiner::RegisterOptions() {
    DESBORDANTE_OPTION_USING;

    auto check_minsup = [](double val) {
        if (val <= 0 || val > 1) {
            throw config::ConfigurationError(
                    "Minimum support must be a value between 0 (exclusive) and 1 (inclusive).");
        }
    };

    auto check_minconf = [](double val) {
        if (val <= 0 || val > 1) {
            throw config::ConfigurationError(
                    "Minimum confidence must be a value between 0 (exclusive) and 1 (inclusive).");
        }
    };

    RegisterOption(config::Option<std::string>{&input_path_, config::names::kTable,
                                               "Path to input file with sequences", std::string{}});

    RegisterOption(config::Option<double>{&min_support_, config::names::kMinimumSupport,
                                          "Minimum support threshold", 0.0}
                           .SetValueCheck(check_minsup));

    RegisterOption(config::Option<double>{&min_confidence_, config::names::kMinimumConfidence,
                                          "Minimum confidence threshold", 0.0}
                           .SetValueCheck(check_minconf));

    RegisterOption(config::Option<std::string>{&output_path_, "output", "Path to output file",
                                               std::string{}});
}

void AlgoERMiner::LoadDataInternal() {
    database_->LoadFile(input_path_);
}

void AlgoERMiner::ResetState() {
    ruleCount_ = 0;
    totalCandidateCount_ = 0;
    candidatePrunedCount_ = 0;
    mapItemCount_.clear();
    database_.reset(new SequenceDatabase());
    matrix_ = SparseMatrix();
    store_ = ExpandLeftStore();
    discoveredRules_.clear();
}

unsigned long long AlgoERMiner::ExecuteInternal() {
    auto start = std::chrono::high_resolution_clock::now();

    RunAlgorithm(min_support_, min_confidence_, input_path_, output_path_);

    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

std::string Rule::ToString() const {
    std::stringstream ss;

    ss << "[";
    for (size_t i = 0; i < antecedent.size(); i++) {
        ss << antecedent[i];
        if (i != antecedent.size() - 1) {
            ss << ", ";
        }
    }

    ss << "] ==> [";

    for (size_t i = 0; i < consequent.size(); i++) {
        ss << consequent[i];
        if (i != consequent.size() - 1) {
            ss << ", ";
        }
    }

    ss << "]  (support: " << support << ", confidence: " << confidence << ")";

    return ss.str();
}

void AlgoERMiner::RunAlgorithm(double minSupport, double minConfidence, std::string const& input,
                               std::string const& output) {
    try {
        database_ = std::make_unique<SequenceDatabase>();
        database_->LoadFile(input);
    } catch (std::exception const& e) {
        std::cerr << "Error loading database: " << e.what() << std::endl;
        return;
    }

    this->minsuppRelative_ = static_cast<int>(std::ceil(minSupport * database_->Size()));
    RunAlgorithm(input, output, minsuppRelative_, minConfidence);
}

void AlgoERMiner::RunAlgorithm(std::string const& input, std::string const& output,
                               int relativeMinsup, double minConfidence) {
    this->minConfidence_ = minConfidence;
    ruleCount_ = 0;

    if (!database_) {
        try {
            database_ = std::make_unique<SequenceDatabase>();
            database_->LoadFile(input);
        } catch (std::exception const& e) {
            std::cerr << "Error loading database: " << e.what() << std::endl;
            return;
        }
    }

    writer_.open(output);
    if (!writer_.is_open()) {
        std::cerr << "Cannot open output file: " << output << std::endl;
        return;
    }

    this->minsuppRelative_ = relativeMinsup;
    if (this->minsuppRelative_ == 0) {
        this->minsuppRelative_ = 1;
    }

    timeStart_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();

    if (maxAntecedentSize_ > 0 && maxConsequentSize_ > 0) {
        CalculateFrequencyOfEachItem();
        GenerateMatrix();
    }

    std::unordered_map<int, LeftEquivalenceClass> map_eclass_left;
    std::unordered_map<int, RightEquivalenceClass> map_eclass_right;

    for (auto const& entry : matrix_.matrix) {
        int int_i = entry.first;
        auto const& occurrences_i = mapItemCount_[int_i];

        if (occurrences_i.size() < static_cast<size_t>(minsuppRelative_)) {
            continue;
        }

        std::vector<int> tids_i;
        for (auto const& occ : occurrences_i) {
            tids_i.push_back(occ.first);
        }
        std::sort(tids_i.begin(), tids_i.end());

        for (auto const& entry_j : entry.second) {
            int int_j = entry_j.first;
            int support_ij = entry_j.second;

            if (support_ij < minsuppRelative_) {
                continue;
            }

            auto const& occurrences_j = mapItemCount_[int_j];

            if (occurrences_j.size() < static_cast<size_t>(minsuppRelative_)) {
                continue;
            }

            std::vector<int> tids_j;
            for (auto const& occ : occurrences_j) {
                tids_j.push_back(occ.first);
            }
            std::sort(tids_j.begin(), tids_j.end());

            std::vector<int> tids_ij;
            std::vector<int> tids_ji;

            CalculateTidsetsIJandJi(occurrences_i, occurrences_j, tids_ij, tids_ji);

            if (tids_ij.size() >= static_cast<size_t>(minsuppRelative_)) {
                double conf_ij = static_cast<double>(tids_ij.size()) / occurrences_i.size();
                std::vector<int> itemset_i = {int_i};
                std::vector<int> itemset_j = {int_j};

                if (conf_ij >= minConfidence) {
                    SaveRule(tids_ij, conf_ij, itemset_i, itemset_j);
                }

                if (maxAntecedentSize_ > 1 || maxConsequentSize_ > 1) {
                    RegisterRule11(int_i, int_j, tids_i, tids_j, tids_ij, occurrences_i,
                                   occurrences_j, map_eclass_left, map_eclass_right);
                }
            }

            if (tids_ji.size() >= static_cast<size_t>(minsuppRelative_)) {
                double conf_ji = static_cast<double>(tids_ji.size()) / occurrences_j.size();
                std::vector<int> itemset_i = {int_i};
                std::vector<int> itemset_j = {int_j};

                if (conf_ji >= minConfidence) {
                    SaveRule(tids_ji, conf_ji, itemset_j, itemset_i);
                }

                if (maxAntecedentSize_ > 1 || maxConsequentSize_ > 1) {
                    RegisterRule11(int_j, int_i, tids_j, tids_i, tids_ji, occurrences_j,
                                   occurrences_i, map_eclass_left, map_eclass_right);
                }
            }
        }
    }

    if (maxAntecedentSize_ > 1) {
        for (auto& eclass_left_pair : map_eclass_left) {
            auto& eclass_left = eclass_left_pair.second;
            if (eclass_left.rules.size() > 1) {
                std::vector<LeftRule> rules_vec(eclass_left.rules.begin(), eclass_left.rules.end());
                std::sort(rules_vec.begin(), rules_vec.end(),
                          [](LeftRule const& a, LeftRule const& b) {
                              return a.itemsetI[0] < b.itemsetI[0];
                          });

                eclass_left.rules.assign(rules_vec.begin(), rules_vec.end());
                ExpandLeft(eclass_left);
            }
        }
    }

    if (maxConsequentSize_ > 1) {
        for (auto& eclass_right_pair : map_eclass_right) {
            auto& eclass_right = eclass_right_pair.second;
            if (eclass_right.rules.size() > 1) {
                std::vector<RightRule> rules_vec(eclass_right.rules.begin(),
                                                 eclass_right.rules.end());
                std::sort(rules_vec.begin(), rules_vec.end(),
                          [](RightRule const& a, RightRule const& b) {
                              return a.itemsetJ[0] < b.itemsetJ[0];
                          });

                eclass_right.rules.assign(rules_vec.begin(), rules_vec.end());
                ExpandRight(eclass_right);
            }
        }
    }

    for (auto& size_map : store_.store) {
        for (auto& hash_list_pair : size_map.second) {
            for (auto& eclass : hash_list_pair.second) {
                if (eclass.rules.size() > 1) {
                    std::vector<LeftRule> rules_vec(eclass.rules.begin(), eclass.rules.end());
                    std::sort(rules_vec.begin(), rules_vec.end(),
                              [](LeftRule const& a, LeftRule const& b) {
                                  return a.itemsetI.back() < b.itemsetI.back();
                              });

                    eclass.rules.assign(rules_vec.begin(), rules_vec.end());
                    ExpandLeft(eclass);
                }
            }
        }
    }

    timeEnd_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count();

    writer_.close();
    database_.reset();
}

void AlgoERMiner::CalculateTidsetsIJandJi(std::unordered_map<int, Occurrence> const& occurrencesI,
                                          std::unordered_map<int, Occurrence> const& occurrencesJ,
                                          std::vector<int>& tidsIJ,
                                          std::vector<int>& tidsJI) const {
    std::vector<int> common_tids;
    if (occurrencesI.size() < occurrencesJ.size()) {
        for (auto const& entry_occ_i : occurrencesI) {
            int tid = entry_occ_i.first;
            if (occurrencesJ.find(tid) != occurrencesJ.end()) {
                common_tids.push_back(tid);
            }
        }
    } else {
        for (auto const& entry_occ_j : occurrencesJ) {
            int tid = entry_occ_j.first;
            if (occurrencesI.find(tid) != occurrencesI.end()) {
                common_tids.push_back(tid);
            }
        }
    }

    for (int tid : common_tids) {
        Occurrence const& occ_i = occurrencesI.at(tid);
        Occurrence const& occ_j = occurrencesJ.at(tid);

        if (occ_i.firstItemset < occ_j.lastItemset) {
            tidsIJ.push_back(tid);
        }
        if (occ_j.firstItemset < occ_i.lastItemset) {
            tidsJI.push_back(tid);
        }
    }

    std::sort(tidsIJ.begin(), tidsIJ.end());
    tidsIJ.erase(std::unique(tidsIJ.begin(), tidsIJ.end()), tidsIJ.end());

    std::sort(tidsJI.begin(), tidsJI.end());
    tidsJI.erase(std::unique(tidsJI.begin(), tidsJI.end()), tidsJI.end());
}

void AlgoERMiner::CalculateFrequencyOfEachItem() {
    mapItemCount_.clear();

    for (size_t k = 0; k < database_->GetSequences().size(); k++) {
        auto const& sequence = database_->GetSequences()[k];

        for (int j = 0; j < sequence->Size(); j++) {
            auto const& itemset = sequence->Get(j);

            for (int item_i : itemset) {
                auto& occurrences = mapItemCount_[item_i];
                auto it = occurrences.find(k);
                if (it == occurrences.end()) {
                    Occurrence occ(static_cast<int16_t>(j), static_cast<int16_t>(j));
                    occurrences.emplace(k, occ);
                } else {
                    it->second.lastItemset = static_cast<int16_t>(j);
                }
            }
        }
    }
}

void AlgoERMiner::RegisterRule11(int intI, int intJ, std::vector<int> const& tidsI,
                                 std::vector<int> const& tidsJ, std::vector<int> const& tidsIJ,
                                 std::unordered_map<int, Occurrence> const& occurrencesI,
                                 std::unordered_map<int, Occurrence> const& occurrencesJ,
                                 std::unordered_map<int, LeftEquivalenceClass>& mapEclassLeft,
                                 std::unordered_map<int, RightEquivalenceClass>& mapEclassRight) {
    auto left_it = mapEclassLeft.find(intJ);
    if (left_it == mapEclassLeft.end()) {
        mapEclassLeft.emplace(intJ, LeftEquivalenceClass({intJ}, tidsJ, occurrencesJ));
        left_it = mapEclassLeft.find(intJ);
    }
    left_it->second.rules.emplace_back(std::vector<int>{intI}, tidsI, tidsIJ);

    auto right_it = mapEclassRight.find(intI);
    if (right_it == mapEclassRight.end()) {
        mapEclassRight.emplace(intI, RightEquivalenceClass({intI}, tidsI, occurrencesI));
        right_it = mapEclassRight.find(intI);
    }
    right_it->second.rules.emplace_back(std::vector<int>{intJ}, tidsJ, tidsIJ, occurrencesJ);
}

std::vector<int> AlgoERMiner::Concatenate(std::vector<int> const& itemset, int item) const {
    std::vector<int> new_itemset = itemset;
    new_itemset.push_back(item);
    return new_itemset;
}

void AlgoERMiner::ExpandLeft(LeftEquivalenceClass& eclass) {
    std::vector<LeftRule> rules_vec(eclass.rules.begin(), eclass.rules.end());

    for (size_t w = 0; w < rules_vec.size() - 1; w++) {
        LeftRule& rule1 = rules_vec[w];
        int d = rule1.itemsetI.back();

        LeftEquivalenceClass rules_for_recursion(eclass.itemsetJ, eclass.tidsJ,
                                                 eclass.occurrencesJ);

        for (size_t m = w + 1; m < rules_vec.size(); m++) {
            LeftRule& rule2 = rules_vec[m];
            int c = rule2.itemsetI.back();

            if (matrix_.GetCount(c, d) < minsuppRelative_) {
                candidatePrunedCount_++;
                totalCandidateCount_++;
                continue;
            }
            totalCandidateCount_++;

            std::vector<int> tids_ic;
            auto const& map_c = mapItemCount_[c];

            std::vector<int> const& sorted_tids_i = rule1.tidsI;

            if (sorted_tids_i.size() < map_c.size()) {
                int remains = static_cast<int>(sorted_tids_i.size());
                for (int tid : sorted_tids_i) {
                    if (map_c.find(tid) != map_c.end()) {
                        tids_ic.push_back(tid);
                    }
                    remains--;
                    if (tids_ic.size() + static_cast<size_t>(remains) <
                        static_cast<size_t>(minsuppRelative_)) {
                        break;
                    }
                }
            } else {
                int remains = static_cast<int>(map_c.size());
                for (auto const& entry : map_c) {
                    int tid = entry.first;
                    if (std::binary_search(sorted_tids_i.begin(), sorted_tids_i.end(), tid)) {
                        tids_ic.push_back(tid);
                    }
                    remains--;
                    if (tids_ic.size() + static_cast<size_t>(remains) <
                        static_cast<size_t>(minsuppRelative_)) {
                        break;
                    }
                }
            }

            std::sort(tids_ic.begin(), tids_ic.end());
            tids_ic.erase(std::unique(tids_ic.begin(), tids_ic.end()), tids_ic.end());

            std::vector<int> tids_ic_j;
            std::vector<int> const& sorted_tids_ij = rule1.tidsIJ;

            if (sorted_tids_ij.size() < map_c.size()) {
                for (int tid : sorted_tids_ij) {
                    auto occurrence_c_it = map_c.find(tid);
                    if (occurrence_c_it != map_c.end()) {
                        auto occurrence_j_it = eclass.occurrencesJ.find(tid);
                        if (occurrence_j_it != eclass.occurrencesJ.end()) {
                            if (occurrence_c_it->second.firstItemset <
                                occurrence_j_it->second.lastItemset) {
                                tids_ic_j.push_back(tid);
                            }
                        }
                    }
                }
            } else {
                for (auto const& entry_c : map_c) {
                    int tid = entry_c.first;
                    if (std::binary_search(sorted_tids_ij.begin(), sorted_tids_ij.end(), tid)) {
                        auto occurrence_j_it = eclass.occurrencesJ.find(tid);
                        if (occurrence_j_it != eclass.occurrencesJ.end()) {
                            if (entry_c.second.firstItemset < occurrence_j_it->second.lastItemset) {
                                tids_ic_j.push_back(tid);
                            }
                        }
                    }
                }
            }

            std::sort(tids_ic_j.begin(), tids_ic_j.end());
            tids_ic_j.erase(std::unique(tids_ic_j.begin(), tids_ic_j.end()), tids_ic_j.end());

            if (tids_ic_j.size() >= static_cast<size_t>(minsuppRelative_)) {
                double conf_ic_j = static_cast<double>(tids_ic_j.size()) / tids_ic.size();
                std::vector<int> itemset_ic = Concatenate(rule1.itemsetI, c);

                if (conf_ic_j >= minConfidence_) {
                    SaveRule(tids_ic_j, conf_ic_j, itemset_ic, eclass.itemsetJ);
                }

                if (static_cast<int>(itemset_ic.size()) < maxAntecedentSize_) {
                    rules_for_recursion.rules.emplace_back(itemset_ic, tids_ic, tids_ic_j);
                }
            }
        }

        if (rules_for_recursion.rules.size() > 1) {
            ExpandLeft(rules_for_recursion);
        }
    }
}

void AlgoERMiner::ExpandRight(RightEquivalenceClass& eclass) {
    std::vector<RightRule> rules_vec(eclass.rules.begin(), eclass.rules.end());

    for (size_t w = 0; w < rules_vec.size() - 1; w++) {
        RightRule& rule1 = rules_vec[w];
        int d = rule1.itemsetJ.back();

        RightEquivalenceClass rules_for_recursion(eclass.itemsetI, eclass.tidsI,
                                                  eclass.occurrencesI);

        for (size_t m = w + 1; m < rules_vec.size(); m++) {
            RightRule& rule2 = rules_vec[m];
            int c = rule2.itemsetJ.back();

            if (matrix_.GetCount(c, d) < minsuppRelative_) {
                candidatePrunedCount_++;
                totalCandidateCount_++;
                continue;
            }
            totalCandidateCount_++;

            std::vector<int> tids_i_jc;
            auto const& map_c = mapItemCount_[c];

            std::vector<int> const& sorted_tids_ij = rule1.tidsIJ;

            if (sorted_tids_ij.size() < map_c.size()) {
                int remains = static_cast<int>(sorted_tids_ij.size());
                for (int tid : sorted_tids_ij) {
                    auto occurrence_c_it = map_c.find(tid);
                    if (occurrence_c_it != map_c.end()) {
                        auto occurrence_i_it = eclass.occurrencesI.find(tid);
                        if (occurrence_i_it != eclass.occurrencesI.end()) {
                            if (occurrence_c_it->second.lastItemset >
                                occurrence_i_it->second.firstItemset) {
                                tids_i_jc.push_back(tid);
                            }
                        }
                    }
                    remains--;
                    if (tids_i_jc.size() + static_cast<size_t>(remains) <
                        static_cast<size_t>(minsuppRelative_)) {
                        break;
                    }
                }
            } else {
                int remains = static_cast<int>(map_c.size());
                for (auto const& entry_c : map_c) {
                    int tid = entry_c.first;
                    if (std::binary_search(sorted_tids_ij.begin(), sorted_tids_ij.end(), tid)) {
                        auto occurrence_i_it = eclass.occurrencesI.find(tid);
                        if (occurrence_i_it != eclass.occurrencesI.end()) {
                            if (entry_c.second.lastItemset > occurrence_i_it->second.firstItemset) {
                                tids_i_jc.push_back(tid);
                            }
                        }
                    }
                    remains--;
                    if (tids_i_jc.size() + static_cast<size_t>(remains) <
                        static_cast<size_t>(minsuppRelative_)) {
                        break;
                    }
                }
            }

            std::sort(tids_i_jc.begin(), tids_i_jc.end());
            tids_i_jc.erase(std::unique(tids_i_jc.begin(), tids_i_jc.end()), tids_i_jc.end());

            if (tids_i_jc.size() >= static_cast<size_t>(minsuppRelative_)) {
                std::vector<int> tids_jc;
                std::unordered_map<int, Occurrence> occurrences_jc;

                std::vector<int> const& sorted_tids_j = rule1.tidsJ;

                if (sorted_tids_j.size() < map_c.size()) {
                    for (int tid : sorted_tids_j) {
                        auto occurrence_c_it = map_c.find(tid);
                        if (occurrence_c_it != map_c.end()) {
                            tids_jc.push_back(tid);
                            auto occurrence_j_it = rule1.occurrencesJ.find(tid);
                            if (occurrence_j_it != rule1.occurrencesJ.end()) {
                                if (occurrence_c_it->second.lastItemset <
                                    occurrence_j_it->second.lastItemset) {
                                    occurrences_jc[tid] = occurrence_c_it->second;
                                } else {
                                    occurrences_jc[tid] = occurrence_j_it->second;
                                }
                            }
                        }
                    }
                } else {
                    for (auto const& entry_c : map_c) {
                        int tid = entry_c.first;
                        if (std::binary_search(sorted_tids_j.begin(), sorted_tids_j.end(), tid)) {
                            tids_jc.push_back(tid);
                            auto occurrence_j_it = rule1.occurrencesJ.find(tid);
                            if (occurrence_j_it != rule1.occurrencesJ.end()) {
                                if (entry_c.second.lastItemset <
                                    occurrence_j_it->second.lastItemset) {
                                    occurrences_jc[tid] = entry_c.second;
                                } else {
                                    occurrences_jc[tid] = occurrence_j_it->second;
                                }
                            }
                        }
                    }
                }

                std::sort(tids_jc.begin(), tids_jc.end());
                tids_jc.erase(std::unique(tids_jc.begin(), tids_jc.end()), tids_jc.end());

                double conf_i_jc = static_cast<double>(tids_i_jc.size()) / eclass.tidsI.size();
                std::vector<int> itemset_jc = Concatenate(rule1.itemsetJ, c);

                if (conf_i_jc >= minConfidence_) {
                    SaveRule(tids_i_jc, conf_i_jc, eclass.itemsetI, itemset_jc);
                }

                RightRule right_rule(itemset_jc, tids_jc, tids_i_jc, occurrences_jc);

                if (static_cast<int>(itemset_jc.size()) < maxConsequentSize_) {
                    rules_for_recursion.rules.push_back(right_rule);
                }

                if (static_cast<int>(eclass.itemsetI.size()) < maxAntecedentSize_) {
                    LeftRule left_rule(eclass.itemsetI, eclass.tidsI, tids_i_jc);
                    store_.RegisterRule(left_rule, itemset_jc, tids_jc, occurrences_jc);
                }
            }
        }

        if (rules_for_recursion.rules.size() > 1) {
            ExpandRight(rules_for_recursion);
        }
    }
}

void AlgoERMiner::GenerateMatrix() {
    for (auto const& sequence_ptr : database_->GetSequences()) {
        auto const& sequence = *sequence_ptr;
        std::unordered_set<int> already_processed;

        for (auto const& itemsetj : sequence.GetItemsets()) {
            for (int itemk : itemsetj) {
                if (already_processed.find(itemk) != already_processed.end() ||
                    mapItemCount_[itemk].size() < static_cast<size_t>(minsuppRelative_)) {
                    continue;
                }

                std::unordered_set<int> already_processed_with_respect_to_k;
                for (auto const& itemsetjj : sequence.GetItemsets()) {
                    for (int itemkk : itemsetjj) {
                        if (itemkk == itemk ||
                            already_processed_with_respect_to_k.find(itemkk) !=
                                    already_processed_with_respect_to_k.end() ||
                            mapItemCount_[itemkk].size() < static_cast<size_t>(minsuppRelative_)) {
                            continue;
                        }

                        matrix_.IncreaseCountOfPair(itemk, itemkk);
                        already_processed_with_respect_to_k.insert(itemkk);
                    }
                }
                already_processed.insert(itemk);
            }
        }
    }
}

void AlgoERMiner::SaveRule(std::vector<int> const& tidsIJ, double confIJ,
                           std::vector<int> const& itemsetI, std::vector<int> const& itemsetJ) {
    ruleCount_++;

    Rule rule;
    rule.antecedent = itemsetI;
    rule.consequent = itemsetJ;
    rule.support = tidsIJ.size();
    rule.confidence = confIJ;
    discoveredRules_.push_back(rule);

    std::stringstream buffer;
    for (size_t i = 0; i < itemsetI.size(); i++) {
        buffer << itemsetI[i];
        if (i != itemsetI.size() - 1) {
            buffer << ",";
        }
    }

    buffer << " ==> ";

    for (size_t i = 0; i < itemsetJ.size(); i++) {
        buffer << itemsetJ[i];
        if (i != itemsetJ.size() - 1) {
            buffer << ",";
        }
    }

    buffer << " #SUP: " << tidsIJ.size();
    buffer << " #CONF: " << confIJ;

    try {
        writer_ << buffer.str() << "\n";
    } catch (std::exception const& e) {
        std::cerr << "Error writing rule: " << e.what() << std::endl;
    }
}

std::vector<Rule> AlgoERMiner::GetRules() const {
    return discoveredRules_;
}

void AlgoERMiner::SetMaxAntecedentSize(int size) {
    maxAntecedentSize_ = size;
}

void AlgoERMiner::SetMaxConsequentSize(int size) {
    maxConsequentSize_ = size;
}

void AlgoERMiner::PrintStats() const {
    std::cout << "=============  ERMiner - STATS ========" << std::endl;
    std::cout << "Sequential rules count: " << ruleCount_ << std::endl;
    std::cout << "Total time: " << (timeEnd_ - timeStart_) << " ms" << std::endl;
    std::cout << "Candidates pruned: " << candidatePrunedCount_ << " of " << totalCandidateCount_
              << std::endl;
    std::cout << "==========================================" << std::endl;
}
}  // namespace algos