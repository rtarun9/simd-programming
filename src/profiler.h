#ifndef __PROFILER_H__
#define __PROFILER_H__

#include "common.h"

#include <emmintrin.h>
#include <immintrin.h>

#define PROFILER_ITERATIONS 25

// When using rtdsc, it is often better to perform multiple iterations and only take the lowest value.
#if defined(_MSC_VER)
#define BEGIN_COUNTER(id)                                                                                              \
    _mm_mfence();                                                                                                      \
    i64 counter_##id = __rdtsc();
#define END_AND_RETURN_COUNTER(id) __rdtsc() - counter_##id;

#else
#error "Only MSVC is supported for now!!"
#endif

#endif
