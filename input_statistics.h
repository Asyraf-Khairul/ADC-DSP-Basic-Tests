#pragma once
#include <cstdint>
struct InputStatistics {
    uint16_t lo[2]{}, hi[2]{}, mean[2]{}, rms[2]{};
    uint32_t near_rail = 0;
};
void collect_input_statistics(uint16_t *raw, InputStatistics &stats);
