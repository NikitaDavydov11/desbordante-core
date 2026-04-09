#include "Occurence.hpp"

Occurrence::Occurrence() : firstItemset(0), lastItemset(0) {}

Occurrence::Occurrence(int16_t firstItemset, int16_t lastItemset)
    : firstItemset(firstItemset), lastItemset(lastItemset) {}