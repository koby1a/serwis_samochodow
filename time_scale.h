#pragma once
/** @file time_scale.h */

#include <unistd.h>
#include <cstdint>
#include "serwis_ipc.h"

static inline int serwis_norm_time_scale(int scale) {
    if (scale <= 0) return 1;
    return scale;
}

static inline int serwis_time_scale_effective(int scale) {
    if (scale > 0) return scale;
    return 1;
}

static inline void serwis_sleep_us_scaled(long long us, int scale) {
    int s = serwis_time_scale_effective(scale);
    long long scaled = us / s;
    if (scaled < 0) scaled = 0;
    usleep((useconds_t)scaled);
}

static inline void serwis_sleep_ms_scaled(long long ms, int scale) {
    serwis_sleep_us_scaled(ms * 1000LL, scale);
}

static inline void serwis_sleep_us_global(long long us) {
    serwis_sleep_us_scaled(us, serwis_time_scale_get());
}

static inline void serwis_sleep_ms_global(long long ms) {
    serwis_sleep_ms_scaled(ms, serwis_time_scale_get());
}
