// (c) Electronic Arts. All Rights Reserved.

#include "nn/os.h"

inline uint64_t get_cycle_counter64()
{
    return nn::os::Tick::GetSystemCurrent();
}

inline uint64_t get_cycles_per_second()
{
    return nn::os::Tick::TICKS_PER_SECOND;
}

