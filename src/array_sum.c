#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "profiler.h"

#define ARRAY_SIZE 1024 * 32

b32 are_arrays_equal(i32 *restrict a, i32 *restrict b, i64 size)
{
    for (i64 i = 0; i < size; i++)
    {
        if (a[i] != b[i])
        {
            return 0;
        }
    }

    return 1;
}

void reset_buffer(i32 *buffer, i64 size)
{
    for (i64 i = 0; i < size; i++)
    {
        buffer[i] = 0;
    }
}

int main()
{
    i32 *a = (i32 *)_aligned_malloc(sizeof(i32) * ARRAY_SIZE, 32);
    i32 *b = (i32 *)_aligned_malloc(sizeof(i32) * ARRAY_SIZE, 32);

    i32 *actual_result = (i32 *)_aligned_malloc(sizeof(i32) * ARRAY_SIZE, 32);

    // All array sum approaches will use this buffer.
    i32 *result = (i32 *)_aligned_malloc(sizeof(i32) * ARRAY_SIZE, 32);

    // Initialize all arrays.
    for (i32 i = 0; i < ARRAY_SIZE; i++)
    {
        a[i] = i;
        b[i] = ARRAY_SIZE - i;
        actual_result[i] = a[i] + b[i];

        result[i] = 0;
    }

    {
        // Find the element wise sum of 2 arrays (non simd, element wise).
        i64 element_wise_array_sum_cycles = (i64)INT64_MAX;

        for (i32 counter_iter = 0; counter_iter <= PROFILER_ITERATIONS; counter_iter++)
        {
            reset_buffer(result, ARRAY_SIZE);

            BEGIN_COUNTER(element_wise_array_sum);
            {
                for (i32 i = 0; i < ARRAY_SIZE; i++)
                {
                    result[i] = a[i] + b[i];
                }
            }

            i64 cycles = END_AND_RETURN_COUNTER(element_wise_array_sum);
            element_wise_array_sum_cycles = min(element_wise_array_sum_cycles, cycles);

            if (!are_arrays_equal(result, actual_result, ARRAY_SIZE))
            {
                printf("Array sum failed (normal version) !!!");
                return 1;
            }
        }

        printf("Element wise array sum cycles : %I64d\n", element_wise_array_sum_cycles);
    }

    {
        // Find the sum of 2 arrays (using simd with 128 bit registers, so 4 element at a time).
        i64 simd_array_sum_cycles = (i64)INT64_MAX;
        for (i32 counter_iter = 0; counter_iter <= PROFILER_ITERATIONS; counter_iter++)
        {
            reset_buffer(result, ARRAY_SIZE);

            BEGIN_COUNTER(simd_array_sum);
            {
                for (i32 i = 0; i < ARRAY_SIZE; i += 4)
                {
                    // Load 4 elements from a and b to __m128i's.
                    __m128i simd_a = _mm_load_si128((__m128i *)&a[i]);
                    __m128i simd_b = _mm_load_si128((__m128i *)&b[i]);

                    // Get the sum and store in result.
                    __m128i sum = _mm_add_epi32(simd_a, simd_b);
                    _mm_store_si128((__m128i *)&result[i], sum);
                }
            }
            i64 cycles = END_AND_RETURN_COUNTER(simd_array_sum);
            simd_array_sum_cycles = min(cycles, simd_array_sum_cycles);

            if (!are_arrays_equal(result, actual_result, ARRAY_SIZE))
            {
                printf("Array sum failed (simd 128) !!!");
                return 1;
            }
        }

        printf("SIMD (128 bit registers) array sum cycles : %I64d\n", simd_array_sum_cycles);
    }

    {
        // Find the sum of 2 arrays (using 256 bit simd registers, so 8 element at a time).
        i64 simd_array_sum_cycles = (i64)INT64_MAX;
        for (i32 counter_iter = 0; counter_iter <= PROFILER_ITERATIONS; counter_iter++)
        {
            reset_buffer(result, ARRAY_SIZE);
            BEGIN_COUNTER(simd_array_sum);
            {
                for (i32 i = 0; i < ARRAY_SIZE; i += 8)
                {
                    __m256i simd_a = _mm256_load_si256((__m256i *)&a[i]);
                    __m256i simd_b = _mm256_load_si256((__m256i *)&b[i]);

                    // Get the sum and store in result.
                    __m256i sum = _mm256_add_epi32(simd_a, simd_b);
                    _mm256_store_si256((__m256i *)&result[i], sum);
                }
            }
            i64 cycles = END_AND_RETURN_COUNTER(simd_array_sum);
            simd_array_sum_cycles = min(simd_array_sum_cycles, cycles);
            if (!are_arrays_equal(result, actual_result, ARRAY_SIZE))
            {
                printf("Array sum failed (simd 256) !!!");
                return 1;
            }
        }

        printf("SIMD (256 bit registers) array sum cycles : %I64d\n", simd_array_sum_cycles);
    }

    {
        // Find the sum of 2 arrays (using 256 bit simd registers, so 8 element at a time, with 1 level of loop
        // unrolling).
        i64 simd_array_sum_cycles = (i64)INT64_MAX;
        for (i32 counter_iter = 0; counter_iter <= PROFILER_ITERATIONS; counter_iter++)
        {
            reset_buffer(result, ARRAY_SIZE);
            BEGIN_COUNTER(simd_array_sum);
            {
                for (i32 i = 0; i < ARRAY_SIZE; i += 16)
                {
                    __m256i simd_a = _mm256_load_si256((__m256i *)&a[i]);
                    __m256i simd_b = _mm256_load_si256((__m256i *)&b[i]);

                    // Get the sum and store in result.
                    __m256i sum_1 = _mm256_add_epi32(simd_a, simd_b);

                    simd_a = _mm256_load_si256((__m256i *)&a[i + 8]);
                    simd_b = _mm256_load_si256((__m256i *)&b[i + 8]);

                    __m256i sum_2 = _mm256_add_epi32(simd_a, simd_b);

                    _mm256_store_si256((__m256i *)&result[i], sum_1);
                    _mm256_store_si256((__m256i *)&result[i + 8], sum_2);
                }
            }
            i64 cycles = END_AND_RETURN_COUNTER(simd_array_sum);
            simd_array_sum_cycles = min(simd_array_sum_cycles, cycles);
            if (!are_arrays_equal(result, actual_result, ARRAY_SIZE))
            {
                printf("Array sum failed (loop unrolled simd 256) !!!");
                return 1;
            }
        }

        printf("SIMD (256 bit registers) array sum cycles : %I64d\n", simd_array_sum_cycles);
    }
    _aligned_free(a);
    _aligned_free(b);

    _aligned_free(result);
}
