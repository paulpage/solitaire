#include <time.h>
#include <unistd.h>
#include <stdint.h>

#include "timer.h"

uint64_t get_performance_counter() {
    uint64_t t;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    t = now.tv_sec;
    t *= 1000000000;
    t += now.tv_nsec;
    return t;
}

uint64_t get_performance_frequency() {
    return 1000000000;
}

