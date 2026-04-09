#pragma once
#include <cstdint>

struct Occurrence {
    int16_t firstItemset;
    int16_t lastItemset;

    Occurrence();
    Occurrence(int16_t firstItemset, int16_t lastItemset);
};