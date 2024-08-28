#include <intrin.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "profiler.h"

#define ARRAY_SIZE 1024 * 32

int main()
{
    int *a = (int *)malloc(sizeof(int) * ARRAY_SIZE);
    int *b = (int *)malloc(sizeof(int) * ARRAY_SIZE);

    int *result = (int *)malloc(sizeof(int) * ARRAY_SIZE);

    // Initialize a and b.
    for (i32 i = 0; i < ARRAY_SIZE; i++)
    {
        a[i] = i;
    }

    for (i32 i = 0; i < ARRAY_SIZE; i++)
    {
        b[i] = ARRAY_SIZE - i;
    }

    for (i32 i = 0; i < ARRAY_SIZE; i++)
    {
        result[i] = 0;
    }

    // Find the element wise sum of 2 arrays (element wise).
    BEGIN_COUNTER(element_wise_array_sum);
    {
        for (i32 i = 0; i < ARRAY_SIZE; i++)
        {
            result[i] = a[i] + b[i];
        }
    }

    i64 element_wise_array_sum_counter = END_AND_RETURN_COUNTER(element_wise_array_sum);

    for (i32 i = 0; i < ARRAY_SIZE; i++)
    {
        // Check the result.
        if (a[i] + b[i] != result[i])
        {
            printf("Array sum failed (normal version) !!!");
            return 1;
        }
    }

    // reset result to 0.
    for (i32 i = 0; i < ARRAY_SIZE; i++)
    {
        result[i] = 0;
    }

    // Find the sum of 2 arrays (using simd, so 4 element at a time).
    BEGIN_COUNTER(simd_array_sum);
    {
        for (i32 i = 0; i < ARRAY_SIZE; i += 4)
        {
            // Load 4 elements from a into __m128i's.
            __m128i simd_a = _mm_set_epi32(a[i + 3], a[i + 2], a[i + 1], a[i]);
            __m128i simd_b = _mm_set_epi32(b[i + 3], b[i + 2], b[i + 1], b[i]);

            // Get the sum and store in result.
            __m128i sum = _mm_add_epi32(simd_a, simd_b);
            _mm_store_si128((__m128i *)&result[i], sum);
        }
    }

    i64 simd_array_sum_counter = END_AND_RETURN_COUNTER(simd_array_sum);

    for (i32 i = 0; i < ARRAY_SIZE; i++)
    {
        // Check the result.
        if (a[i] + b[i] != result[i])
        {
            printf("Array sum failed (simd version) !!!");
            return 1;
        }
    }

    printf("Element wise array sum cycles : %I64d\n", element_wise_array_sum_counter);
    printf("SIMD array sum cycles : %I64d\n", simd_array_sum_counter);

    free(a);
    free(b);

    free(result);
}
