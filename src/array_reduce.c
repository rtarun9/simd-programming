#include <emmintrin.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "profiler.h"

#define ARRAY_SIZE 1024 * 1024

int main()
{
    int *a = (int *)_aligned_malloc(sizeof(int) * ARRAY_SIZE, 32);
    i64 actual_result = 0;

    // Initialize a.
    for (i32 i = 0; i < ARRAY_SIZE; i++)
    {
        a[i] = i;
        actual_result += a[i];
    }

    {
        i64 result = 0;

        // Reduce (i.e sum of elements) using non simd code.
        i64 element_wise_array_reduce_counter = (i64)INT64_MAX;

        for (i32 counter_iter = 0; counter_iter <= PROFILER_ITERATIONS; counter_iter++)
        {
            BEGIN_COUNTER(element_wise_array_reduce);
            {
                for (i32 i = 0; i < ARRAY_SIZE; i++)
                {
                    result += a[i];
                }
            }

            i64 element_wise_array_reduce_counter_for_iter = END_AND_RETURN_COUNTER(element_wise_array_reduce);
            element_wise_array_reduce_counter =
                min(element_wise_array_reduce_counter, element_wise_array_reduce_counter_for_iter);
        }

        if (actual_result != result)
        {
            printf("Array reduction failed (normal version) !!!");
            return 1;
        }
        printf("Element wise array sum cycles : %I64d\n", element_wise_array_reduce_counter);
    }

    {
        i64 result = 0;

        // Find the sum of all array elements (reduction) (using simd with 128 bit registers, so 4 element at a time).
        i64 simd_array_reduce_counter = (i64)INT64_MAX;
        for (i32 counter_iter = 0; counter_iter <= PROFILER_ITERATIONS; counter_iter++)
        {
            __m128i register_sum = _mm_setzero_si128();

            BEGIN_COUNTER(simd_array_reduce);
            {

                for (i32 i = 0; i < ARRAY_SIZE; i += 4)
                {
                    // Load 4 elements from a into __m128i's.
                    __m128i simd_a = _mm_load_si128((__m128i *)&a[i]);
                    register_sum = _mm_add_epi32(simd_a, register_sum);
                }
            }

            // Get the result and sum from register_sum.
            i32 register_value[8] = {0};
            _mm_store_epi32(register_value, register_sum);

            for (i32 i = 0; i < 8; i++)
            {
                result += register_value[i];
            }

            i64 simd_array_reduce_counter_for_iter = END_AND_RETURN_COUNTER(simd_array_sum);
            simd_array_reduce_counter = min(simd_array_sum_counter_for_iter, simd_array_sum_counter);
        }

        // Check the result.
        if (result != actual_result)
        {
            printf("Array reduce failed (simd version) !!!");
            return 1;
        }

        printf("SIMD (128 bit registers) array reduce cycles : %I64d\n", simd_array_reduce_counter);
    }

    // reset result to 0.
    for (i32 i = 0; i < ARRAY_SIZE; i++)
    {
        result[i] = 0;
    }

    {
        // Find the sum of 2 arrays (using 256 bit simd registers, so 8 element at a time).
        i64 simd_array_sum_counter = (i64)INT64_MAX;
        for (i32 counter_iter = 0; counter_iter <= PROFILER_ITERATIONS; counter_iter++)
        {
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
            i64 simd_array_sum_counter_for_iter = END_AND_RETURN_COUNTER(simd_array_sum);
            simd_array_sum_counter = min(simd_array_sum_counter_for_iter, simd_array_sum_counter);
        }

        for (i32 i = 0; i < ARRAY_SIZE; i++)
        {
            // Check the result.
            if (a[i] + b[i] != result[i])
            {
                printf("Array sum failed (simd version) !!!");
                return 1;
            }
        }

        printf("SIMD (256 bit registers) array sum cycles : %I64d\n", simd_array_sum_counter);
    }

    _aligned_free(a);
    _aligned_free(b);
}
