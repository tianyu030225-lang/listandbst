#include <stdio.h>
#include <stdlib.h>

#include "tree_bst.h"

#define EXPECT_TRUE(cond)                                                          \
    do                                                                             \
    {                                                                              \
        if (!(cond))                                                               \
        {                                                                          \
            fprintf(stderr, "Expectation failed: %s (line %d)\n", #cond, __LINE__); \
            return EXIT_FAILURE;                                                   \
        }                                                                          \
    } while (0)

static void collect_inorder(const BSTNode *root, ListData *values, size_t *count)
{
    if (root == NULL)
    {
        return;
    }

    collect_inorder(root->left, values, count);
    values[(*count)++] = root->data;
    collect_inorder(root->right, values, count);
}

static int expect_sequence(const ListData *values, const ListData *expected, size_t count)
{
    size_t i;

    for (i = 0U; i < count; ++i)
    {
        EXPECT_TRUE(values[i] == expected[i]);
    }

    return EXIT_SUCCESS;
}

static int test_bst(void)
{
    BSTNode *root = NULL;
    ListData values[] = {5, 3, 7, 1, 4, 6, 8};
    ListData after_insert[] = {1, 3, 4, 5, 6, 7, 8};
    ListData after_delete[] = {4, 6, 7, 8};
    ListData buffer[16] = {0};
    size_t count = 0U;
    size_t i;

    EXPECT_TRUE(creat_tree_bst(&root, values[0]));
    for (i = 1U; i < sizeof(values) / sizeof(values[0]); ++i)
    {
        EXPECT_TRUE(add_node_tree_bst(&root, values[i]));
    }

    EXPECT_TRUE(find_node_tree_bst(root, 4) != NULL);
    EXPECT_TRUE(find_node_tree_bst(root, 99) == NULL);

    printf_tree_bst_mid(root);
    printf_tree_bst_front(root);
    printf_tree_bst_back(root);

    collect_inorder(root, buffer, &count);
    EXPECT_TRUE(count == 7U);
    EXPECT_TRUE(expect_sequence(buffer, after_insert, 7U) == EXIT_SUCCESS);

    EXPECT_TRUE(del_node_tree_bst(&root, 1));
    EXPECT_TRUE(del_node_tree_bst(&root, 3));
    EXPECT_TRUE(del_node_tree_bst(&root, 5));

    count = 0U;
    collect_inorder(root, buffer, &count);
    EXPECT_TRUE(count == 4U);
    EXPECT_TRUE(expect_sequence(buffer, after_delete, 4U) == EXIT_SUCCESS);

    EXPECT_TRUE(del_all_node_tree_bst(&root));
    EXPECT_TRUE(root == NULL);
    return EXIT_SUCCESS;
}

int main(void)
{
    if (test_bst() != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    printf("bst tests passed\n");
    return EXIT_SUCCESS;
}
