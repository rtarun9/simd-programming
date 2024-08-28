#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

#define internal static
#define global_variable static
#define local_persist static

#if defined(SIMD_DEBUG)
#define ASSERT(x)                                                                                                      \
    if (!x)                                                                                                            \
    {                                                                                                                  \
        i32 *null_ptr = 0;                                                                                             \
        *null_ptr = 0;                                                                                                 \
    }
#else
#define ASSERT(x)
#endif

#endif
