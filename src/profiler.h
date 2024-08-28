#ifndef __PROFILER_H__
#define __PROFILER_H__

#include "common.h"

#if defined(_MSC_VER)
#define BEGIN_COUNTER(id) i64 counter_##id = __rdtsc();
#define END_AND_RETURN_COUNTER(id) __rdtsc() - counter_##id;
#else
#error "Only MSVC is supported for now!!"
#endif

#endif
