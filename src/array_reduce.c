#include <emmintrin.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "profiler.h"

#define ARRAY_SIZE 1024 * 32 * 32

int main()
{
    int *a = (int *)_aligned_malloc(sizeof(int) * ARRAY_SIZE, 32);
    i64 actual_result = 0;

    for (i32 i = 0; i < ARRAY_SIZE; i++)
    {
        a[i] = i % 16;
        actual_result += a[i];
    }

    // Reduce (i.e sum of elements) using non simd code.
    {
        i64 result = 0;

        i64 element_wise_array_reduce_cycles = (i64)INT64_MAX;

        for (i32 counter_iter = 0; counter_iter <= PROFILER_ITERATIONS; counter_iter++)
        {
            result = 0;

            BEGIN_COUNTER(element_wise_array_reduce);
            {
                for (i32 i = 0; i < ARRAY_SIZE; i++)
                {
                    result += a[i];
                }
            }

            i64 cycles = END_AND_RETURN_COUNTER(element_wise_array_reduce);
            element_wise_array_reduce_cycles = min(cycles, element_wise_array_reduce_cycles);

            if (actual_result != result)
            {
                printf("Array reduction failed (normal version) !!! %I64d %I64d", actual_result, result);
                return 1;
            }
        }

        printf("Element wise array sum cycles : %I64d\n", element_wise_array_reduce_cycles);
    }

    {
        // Find the sum of all array elements (reduction) (using simd with 256 bit registers, so 8 element at a time).
        i64 result = 0;

        i64 simd_array_reduce_cycles = (i64)INT64_MAX;
        for (i32 counter_iter = 0; counter_iter <= PROFILER_ITERATIONS; counter_iter++)
        {
            __m256i register_sum = _mm256_setzero_si256();

            result = 0;

            BEGIN_COUNTER(simd_array_reduce);
            {

                for (i32 i = 0; i < ARRAY_SIZE; i += 16)
                {
                    __m256i simd_a = _mm256_load_si256((__m256i *)&a[i]);
                    register_sum = _mm256_add_epi64(simd_a, register_sum);

                    simd_a = _mm256_load_si256((__m256i *)&a[i + 8]);
                    register_sum = _mm256_add_epi64(simd_a, register_sum);
                }
            }

            // Get the result and sum from register_sum.
            i32 register_value[8] = {0};
            _mm256_store_si256((void *)register_value, register_sum);

            for (i32 i = 0; i < 8; i++)
            {
                result += register_value[i];
            }

            i64 cycles = END_AND_RETURN_COUNTER(simd_array_reduce);
            simd_array_reduce_cycles = min(cycles, simd_array_reduce_cycles);

            if (result != actual_result)
            {
                printf("Array reduce failed (simd 256 version) !!!");
                return 1;
            }
        }

        printf("SIMD (256 bit registers) array reduce cycles : %I64d\n", simd_array_reduce_cycles);
    }

    _aligned_free(a);
}
