#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "runtime_store.h"

#define EXPECT_TRUE(cond)                                                          \
    do                                                                             \
    {                                                                              \
        if (!(cond))                                                               \
        {                                                                          \
            fprintf(stderr, "Expectation failed: %s (line %d)\n", #cond, __LINE__); \
            return EXIT_FAILURE;                                                   \
        }                                                                          \
    } while (0)

static bool append_value(int **values, size_t *count, size_t *capacity, int value)
{
    int *new_values;
    size_t new_capacity;

    if (*count == *capacity)
    {
        new_capacity = *capacity == 0U ? 8U : *capacity * 2U;
        new_values = (int *)realloc(*values, new_capacity * sizeof(*new_values));
        if (new_values == NULL)
        {
            return false;
        }
        *values = new_values;
        *capacity = new_capacity;
    }

    (*values)[(*count)++] = value;
    return true;
}

static int compare_ints(const void *lhs, const void *rhs)
{
    const int *left = (const int *)lhs;
    const int *right = (const int *)rhs;
    return (*left > *right) - (*left < *right);
}

static int compare_multiset(const int *actual,
                            size_t actual_count,
                            const int *expected,
                            size_t expected_count)
{
    int *actual_copy;
    int *expected_copy;

    EXPECT_TRUE(actual_count == expected_count);
    actual_copy = (int *)malloc(actual_count * sizeof(*actual_copy));
    expected_copy = (int *)malloc(expected_count * sizeof(*expected_copy));
    EXPECT_TRUE(actual_copy != NULL);
    EXPECT_TRUE(expected_copy != NULL);

    memcpy(actual_copy, actual, actual_count * sizeof(*actual_copy));
    memcpy(expected_copy, expected, expected_count * sizeof(*expected_copy));

    qsort(actual_copy, actual_count, sizeof(*actual_copy), compare_ints);
    qsort(expected_copy, expected_count, sizeof(*expected_copy), compare_ints);

    EXPECT_TRUE(memcmp(actual_copy, expected_copy, actual_count * sizeof(*actual_copy)) == 0);
    free(actual_copy);
    free(expected_copy);
    return EXIT_SUCCESS;
}

static int collect_slist(const ListNode *head, int **values, size_t *count, size_t *capacity)
{
    const ListNode *node = head;

    while (node != NULL)
    {
        EXPECT_TRUE(append_value(values, count, capacity, node->data));
        node = node->next;
    }

    return EXIT_SUCCESS;
}

static int collect_slist_while(const SListWhileNode *head, int **values, size_t *count, size_t *capacity)
{
    const SListWhileNode *node = head;

    if (head == NULL)
    {
        return EXIT_SUCCESS;
    }

    do
    {
        EXPECT_TRUE(append_value(values, count, capacity, node->data));
        node = node->next;
    } while (node != head);

    return EXIT_SUCCESS;
}

static int collect_dlist(const DListNode *head, int **values, size_t *count, size_t *capacity)
{
    const DListNode *node = head;

    while (node != NULL)
    {
        EXPECT_TRUE(append_value(values, count, capacity, node->data));
        node = node->next;
    }

    return EXIT_SUCCESS;
}

static int collect_dlist_while(const DListWhileNode *head, int **values, size_t *count, size_t *capacity)
{
    const DListWhileNode *node = head;

    if (head == NULL)
    {
        return EXIT_SUCCESS;
    }

    do
    {
        EXPECT_TRUE(append_value(values, count, capacity, node->data));
        node = node->next;
    } while (node != head);

    return EXIT_SUCCESS;
}

static int collect_tree_inorder(const BSTNode *root, int **values, size_t *count, size_t *capacity)
{
    if (root == NULL)
    {
        return EXIT_SUCCESS;
    }

    EXPECT_TRUE(collect_tree_inorder(root->left, values, count, capacity) == EXIT_SUCCESS);
    EXPECT_TRUE(append_value(values, count, capacity, root->data));
    EXPECT_TRUE(collect_tree_inorder(root->right, values, count, capacity) == EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

static bool file_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

static int read_values_from_file(const char *path, int **values, size_t *count, size_t *capacity)
{
    FILE *stream = fopen(path, "r");
    char line[128];

    EXPECT_TRUE(stream != NULL);
    while (fgets(line, sizeof(line), stream) != NULL)
    {
        char *endptr;
        long value = strtol(line, &endptr, 10);
        EXPECT_TRUE(endptr != line);
        EXPECT_TRUE(append_value(values, count, capacity, (int)value));
    }

    EXPECT_TRUE(fclose(stream) == 0);
    return EXIT_SUCCESS;
}

static int populate_state(RuntimeState *state)
{
    EXPECT_TRUE(creat_slist(&state->slist, 1));
    EXPECT_TRUE(tail_add_slist(&state->slist, 2));
    EXPECT_TRUE(tail_add_slist(&state->slist, 3));

    EXPECT_TRUE(creat_slist_while(&state->slist_while, 10));
    EXPECT_TRUE(tail_add_slist_while(&state->slist_while, 20));
    EXPECT_TRUE(tail_add_slist_while(&state->slist_while, 30));

    EXPECT_TRUE(creat_dlist(&state->dlist, 4));
    EXPECT_TRUE(tail_add_dlist(&state->dlist, 5));
    EXPECT_TRUE(tail_add_dlist(&state->dlist, 6));

    EXPECT_TRUE(creat_dlist_while(&state->dlist_while, 40));
    EXPECT_TRUE(tail_add_dlist_while(&state->dlist_while, 50));
    EXPECT_TRUE(tail_add_dlist_while(&state->dlist_while, 60));

    EXPECT_TRUE(creat_tree_bst(&state->tree_bst, 8));
    EXPECT_TRUE(add_node_tree_bst(&state->tree_bst, 3));
    EXPECT_TRUE(add_node_tree_bst(&state->tree_bst, 12));
    EXPECT_TRUE(add_node_tree_bst(&state->tree_bst, 9));
    EXPECT_TRUE(add_node_tree_bst(&state->tree_bst, 15));
    return EXIT_SUCCESS;
}

static int test_save_and_load_roundtrip(void)
{
    RuntimeState state;
    RuntimeState reloaded;
    RuntimeStateReport report;
    int *values = NULL;
    size_t count = 0U;
    size_t capacity = 0U;
    int expected_slist[] = {1, 2, 3};
    int expected_slist_while[] = {10, 20, 30};
    int expected_dlist[] = {4, 5, 6};
    int expected_dlist_while[] = {40, 50, 60};
    int expected_tree[] = {3, 8, 9, 12, 15};

    runtime_state_init(&state);
    runtime_state_init(&reloaded);

    EXPECT_TRUE(populate_state(&state) == EXIT_SUCCESS);
    EXPECT_TRUE(runtime_state_save_with_seed(&state, &report, 7U));
    EXPECT_TRUE(report.files_saved == 5U);
    EXPECT_TRUE(report.values_saved == 17U);

    EXPECT_TRUE(runtime_state_load(&reloaded, &report));
    EXPECT_TRUE(report.values_loaded == 17U);
    EXPECT_TRUE(report.skipped_lines == 0U);

    EXPECT_TRUE(collect_slist(reloaded.slist, &values, &count, &capacity) == EXIT_SUCCESS);
    EXPECT_TRUE(compare_multiset(values, count, expected_slist, sizeof(expected_slist) / sizeof(expected_slist[0])) == EXIT_SUCCESS);
    free(values);
    values = NULL;
    count = 0U;
    capacity = 0U;

    EXPECT_TRUE(collect_slist_while(reloaded.slist_while, &values, &count, &capacity) == EXIT_SUCCESS);
    EXPECT_TRUE(compare_multiset(values, count, expected_slist_while, sizeof(expected_slist_while) / sizeof(expected_slist_while[0])) == EXIT_SUCCESS);
    free(values);
    values = NULL;
    count = 0U;
    capacity = 0U;

    EXPECT_TRUE(collect_dlist(reloaded.dlist, &values, &count, &capacity) == EXIT_SUCCESS);
    EXPECT_TRUE(compare_multiset(values, count, expected_dlist, sizeof(expected_dlist) / sizeof(expected_dlist[0])) == EXIT_SUCCESS);
    free(values);
    values = NULL;
    count = 0U;
    capacity = 0U;

    EXPECT_TRUE(collect_dlist_while(reloaded.dlist_while, &values, &count, &capacity) == EXIT_SUCCESS);
    EXPECT_TRUE(compare_multiset(values, count, expected_dlist_while, sizeof(expected_dlist_while) / sizeof(expected_dlist_while[0])) == EXIT_SUCCESS);
    free(values);
    values = NULL;
    count = 0U;
    capacity = 0U;

    EXPECT_TRUE(collect_tree_inorder(reloaded.tree_bst, &values, &count, &capacity) == EXIT_SUCCESS);
    EXPECT_TRUE(compare_multiset(values, count, expected_tree, sizeof(expected_tree) / sizeof(expected_tree[0])) == EXIT_SUCCESS);

    free(values);
    runtime_state_destroy(&state);
    runtime_state_destroy(&reloaded);
    return EXIT_SUCCESS;
}

static int test_shuffle_preserves_values(void)
{
    RuntimeState state;
    RuntimeStateReport report;
    int *file_values = NULL;
    size_t count = 0U;
    size_t capacity = 0U;
    int expected[] = {1, 2, 3};

    runtime_state_init(&state);
    EXPECT_TRUE(creat_slist(&state.slist, 1));
    EXPECT_TRUE(tail_add_slist(&state.slist, 2));
    EXPECT_TRUE(tail_add_slist(&state.slist, 3));

    EXPECT_TRUE(runtime_state_save_with_seed(&state, &report, 0U));
    EXPECT_TRUE(read_values_from_file("runtime_data/slist.txt", &file_values, &count, &capacity) == EXIT_SUCCESS);
    EXPECT_TRUE(compare_multiset(file_values, count, expected, sizeof(expected) / sizeof(expected[0])) == EXIT_SUCCESS);
    EXPECT_TRUE(!(count == 3U && file_values[0] == 1 && file_values[1] == 2 && file_values[2] == 3));

    free(file_values);
    runtime_state_destroy(&state);
    return EXIT_SUCCESS;
}

static int test_empty_state_and_missing_files(void)
{
    RuntimeState state;
    RuntimeState loaded;
    RuntimeStateReport report;

    runtime_state_init(&state);
    runtime_state_init(&loaded);

    EXPECT_TRUE(runtime_state_save_with_seed(&state, &report, 9U));
    EXPECT_TRUE(file_exists("runtime_data/slist.txt"));
    EXPECT_TRUE(file_exists("runtime_data/tree_bst.txt"));

    EXPECT_TRUE(remove("runtime_data/dlist.txt") == 0);
    EXPECT_TRUE(runtime_state_load(&loaded, &report));
    EXPECT_TRUE(report.files_missing == 1U);
    EXPECT_TRUE(loaded.slist == NULL);
    EXPECT_TRUE(loaded.dlist == NULL);
    EXPECT_TRUE(loaded.tree_bst == NULL);

    runtime_state_destroy(&state);
    runtime_state_destroy(&loaded);
    return EXIT_SUCCESS;
}

static int test_invalid_lines_are_skipped(void)
{
    RuntimeState state;
    RuntimeStateReport report;
    FILE *stream;

    runtime_state_init(&state);
    EXPECT_TRUE(mkdir("runtime_data", 0755) == 0 || errno == EEXIST);

    stream = fopen("runtime_data/slist.txt", "w");
    EXPECT_TRUE(stream != NULL);
    EXPECT_TRUE(fputs("100\nabc\n200\n\n999999999999999999999\n", stream) >= 0);
    EXPECT_TRUE(fclose(stream) == 0);

    stream = fopen("runtime_data/slist_while.txt", "w");
    EXPECT_TRUE(stream != NULL);
    EXPECT_TRUE(fclose(stream) == 0);
    stream = fopen("runtime_data/dlist.txt", "w");
    EXPECT_TRUE(stream != NULL);
    EXPECT_TRUE(fclose(stream) == 0);
    stream = fopen("runtime_data/dlist_while.txt", "w");
    EXPECT_TRUE(stream != NULL);
    EXPECT_TRUE(fclose(stream) == 0);
    stream = fopen("runtime_data/tree_bst.txt", "w");
    EXPECT_TRUE(stream != NULL);
    EXPECT_TRUE(fclose(stream) == 0);

    EXPECT_TRUE(runtime_state_load(&state, &report));
    EXPECT_TRUE(report.skipped_lines == 2U);
    EXPECT_TRUE(find_node_slist(state.slist, 100) != NULL);
    EXPECT_TRUE(find_node_slist(state.slist, 200) != NULL);
    EXPECT_TRUE(find_node_slist(state.slist, 300) == NULL);

    runtime_state_destroy(&state);
    return EXIT_SUCCESS;
}

static int cleanup_tempdir(const char *root, const char *tempdir)
{
    const char *paths[] = {
        "runtime_data/slist.txt",
        "runtime_data/slist_while.txt",
        "runtime_data/dlist.txt",
        "runtime_data/dlist_while.txt",
        "runtime_data/tree_bst.txt",
        "runtime_data",
    };
    size_t i;

    for (i = 0U; i < sizeof(paths) / sizeof(paths[0]); ++i)
    {
        (void)remove(paths[i]);
    }

    EXPECT_TRUE(chdir(root) == 0);
    return rmdir(tempdir);
}

int main(void)
{
    char original_cwd[4096];
    char tempdir[4096];

    EXPECT_TRUE(getcwd(original_cwd, sizeof(original_cwd)) != NULL);
    (void)snprintf(tempdir,
                   sizeof(tempdir),
                   "/tmp/listandtree_runtime_%ld_%ld",
                   (long)getpid(),
                   (long)time(NULL));
    EXPECT_TRUE(mkdir(tempdir, 0700) == 0);
    EXPECT_TRUE(chdir(tempdir) == 0);

    if (test_save_and_load_roundtrip() != EXIT_SUCCESS ||
        test_shuffle_preserves_values() != EXIT_SUCCESS ||
        test_empty_state_and_missing_files() != EXIT_SUCCESS ||
        test_invalid_lines_are_skipped() != EXIT_SUCCESS)
    {
        (void)cleanup_tempdir(original_cwd, tempdir);
        return EXIT_FAILURE;
    }

    EXPECT_TRUE(cleanup_tempdir(original_cwd, tempdir) == 0);
    printf("runtime store tests passed\n");
    return EXIT_SUCCESS;
}
