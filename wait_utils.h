#pragma once

#include <cerrno>
#include <sys/select.h>
#include <sys/time.h>

static inline void serwis_wait_us(long long us) {
    if (us <= 0) return;
    struct timeval tv{};
    tv.tv_sec = (time_t)(us / 1000000LL);
    tv.tv_usec = (suseconds_t)(us % 1000000LL);
    while (select(0, nullptr, nullptr, nullptr, &tv) == -1 && errno == EINTR) {
        // Retry if interrupted by a signal.
    }
}
