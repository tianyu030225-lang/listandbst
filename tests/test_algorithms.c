#include <stdio.h>
#include <stdlib.h>

#include "paixu_search.h"

#define EXPECT_TRUE(cond)                                                          \
    do                                                                             \
    {                                                                              \
        if (!(cond))                                                               \
        {                                                                          \
            fprintf(stderr, "Expectation failed: %s (line %d)\n", #cond, __LINE__); \
            return EXIT_FAILURE;                                                   \
        }                                                                          \
    } while (0)

static int expect_sequence(const ListData *values, const ListData *expected, int count)
{
    int i;

    for (i = 0; i < count; ++i)
    {
        EXPECT_TRUE(values[i] == expected[i]);
    }

    return EXIT_SUCCESS;
}

static int test_bubble_sort(void)
{
    ListData values[] = {7, 2, 9, 4, 1, 5};
    ListData expected[] = {1, 2, 4, 5, 7, 9};
    ListData single[] = {42};

    printf_arr(values, (int)(sizeof(values) / sizeof(values[0])));
    EXPECT_TRUE(bubble_sort(values, (int)(sizeof(values) / sizeof(values[0]))));
    EXPECT_TRUE(expect_sequence(values, expected, (int)(sizeof(expected) / sizeof(expected[0]))) == EXIT_SUCCESS);
    EXPECT_TRUE(bubble_sort(single, 1));
    EXPECT_TRUE(!bubble_sort(NULL, 3));
    return EXIT_SUCCESS;
}

static int test_quick_sort(void)
{
    ListData values[] = {10, 8, 3, 6, 2, 9, 1};
    ListData expected[] = {1, 2, 3, 6, 8, 9, 10};

    EXPECT_TRUE(quick_sort(values, 0, (int)(sizeof(values) / sizeof(values[0])) - 1));
    EXPECT_TRUE(expect_sequence(values, expected, (int)(sizeof(expected) / sizeof(expected[0]))) == EXIT_SUCCESS);
    EXPECT_TRUE(!quick_sort(NULL, 0, 2));
    return EXIT_SUCCESS;
}

static int test_binary_search(void)
{
    ListData values[] = {1, 2, 4, 5, 7, 9};

    EXPECT_TRUE(binary_search(values, (int)(sizeof(values) / sizeof(values[0])), 5) == 3);
    EXPECT_TRUE(binary_search(values, (int)(sizeof(values) / sizeof(values[0])), 8) == -1);
    EXPECT_TRUE(binary_search(NULL, 6, 5) == -1);
    return EXIT_SUCCESS;
}

int main(void)
{
    if (test_bubble_sort() != EXIT_SUCCESS ||
        test_quick_sort() != EXIT_SUCCESS ||
        test_binary_search() != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    printf("algorithm tests passed\n");
    return EXIT_SUCCESS;
}
