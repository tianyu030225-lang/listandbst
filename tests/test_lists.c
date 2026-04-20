#include <stdio.h>
#include <stdlib.h>

#include "dlist.h"
#include "dlist_while.h"
#include "slist.h"
#include "slist_while.h"

#define EXPECT_TRUE(cond)                                                          \
    do                                                                             \
    {                                                                              \
        if (!(cond))                                                               \
        {                                                                          \
            fprintf(stderr, "Expectation failed: %s (line %d)\n", #cond, __LINE__); \
            return EXIT_FAILURE;                                                   \
        }                                                                          \
    } while (0)

static int expect_slist_sequence(const ListNode *head, const ListData *expected, size_t count)
{
    size_t i;
    const ListNode *node = head;

    for (i = 0U; i < count; ++i)
    {
        EXPECT_TRUE(node != NULL);
        EXPECT_TRUE(node->data == expected[i]);
        node = node->next;
    }
    EXPECT_TRUE(node == NULL);
    return EXIT_SUCCESS;
}

static int expect_slist_while_sequence(const SListWhileNode *head, const ListData *expected, size_t count)
{
    size_t i;
    const SListWhileNode *node = head;

    EXPECT_TRUE(head != NULL);
    for (i = 0U; i < count; ++i)
    {
        EXPECT_TRUE(node != NULL);
        EXPECT_TRUE(node->data == expected[i]);
        node = node->next;
    }
    EXPECT_TRUE(node == head);
    return EXIT_SUCCESS;
}

static int expect_dlist_sequence(const DListNode *head, const ListData *expected, size_t count)
{
    size_t i;
    const DListNode *node = head;
    const DListNode *tail = NULL;

    for (i = 0U; i < count; ++i)
    {
        EXPECT_TRUE(node != NULL);
        EXPECT_TRUE(node->data == expected[i]);
        if (node->next != NULL)
        {
            EXPECT_TRUE(node->next->prev == node);
        }
        tail = node;
        node = node->next;
    }
    EXPECT_TRUE(node == NULL);

    for (i = count; i > 0U; --i)
    {
        EXPECT_TRUE(tail != NULL);
        EXPECT_TRUE(tail->data == expected[i - 1U]);
        tail = tail->prev;
    }
    EXPECT_TRUE(tail == NULL);
    return EXIT_SUCCESS;
}

static int expect_dlist_while_sequence(const DListWhileNode *head, const ListData *expected, size_t count)
{
    size_t i;
    const DListWhileNode *node = head;

    EXPECT_TRUE(head != NULL);
    for (i = 0U; i < count; ++i)
    {
        EXPECT_TRUE(node != NULL);
        EXPECT_TRUE(node->data == expected[i]);
        EXPECT_TRUE(node->next->prev == node);
        EXPECT_TRUE(node->prev->next == node);
        node = node->next;
    }
    EXPECT_TRUE(node == head);
    return EXIT_SUCCESS;
}

static int test_slist(void)
{
    ListNode *head = NULL;
    ListData expected_after_insert[] = {0, 1, 2, 3};
    ListData expected_after_update[] = {0, 1, 20, 3};
    ListData expected_after_delete[] = {1, 3};

    EXPECT_TRUE(creat_slist(&head, 1));
    EXPECT_TRUE(head_add_slist(&head, 0));
    EXPECT_TRUE(tail_add_slist(&head, 3));
    EXPECT_TRUE(rand_add_slist(&head, 1, 2));
    printf_slist(head);
    EXPECT_TRUE(expect_slist_sequence(head, expected_after_insert, 4U) == EXIT_SUCCESS);

    EXPECT_TRUE(find_node_slist(head, 2) != NULL);
    EXPECT_TRUE(update_node_slist(head, 2, 20));
    EXPECT_TRUE(expect_slist_sequence(head, expected_after_update, 4U) == EXIT_SUCCESS);

    EXPECT_TRUE(del_node_slist(&head, 20));
    EXPECT_TRUE(del_node_slist(&head, 0));
    EXPECT_TRUE(expect_slist_sequence(head, expected_after_delete, 2U) == EXIT_SUCCESS);

    EXPECT_TRUE(del_all_node_slist(&head));
    EXPECT_TRUE(head == NULL);
    return EXIT_SUCCESS;
}

static int test_slist_while(void)
{
    SListWhileNode *head = NULL;
    SListWhileNode *single = NULL;
    ListData expected_after_insert[] = {0, 1, 2, 3};
    ListData expected_after_update[] = {0, 1, 20, 3};
    ListData expected_after_delete[] = {1, 3};

    EXPECT_TRUE(creat_slist_while(&single, 9));
    EXPECT_TRUE(del_node_slist_while(&single, 9));
    EXPECT_TRUE(single == NULL);

    EXPECT_TRUE(creat_slist_while(&head, 1));
    EXPECT_TRUE(head_add_slist_while(&head, 0));
    EXPECT_TRUE(tail_add_slist_while(&head, 3));
    EXPECT_TRUE(rand_add_slist_while(&head, 1, 2));
    printf_slist_while(head);
    EXPECT_TRUE(expect_slist_while_sequence(head, expected_after_insert, 4U) == EXIT_SUCCESS);

    EXPECT_TRUE(find_node_slist_while(head, 2) != NULL);
    EXPECT_TRUE(update_node_slist_while(head, 2, 20));
    EXPECT_TRUE(expect_slist_while_sequence(head, expected_after_update, 4U) == EXIT_SUCCESS);

    EXPECT_TRUE(del_node_slist_while(&head, 20));
    EXPECT_TRUE(del_node_slist_while(&head, 0));
    EXPECT_TRUE(expect_slist_while_sequence(head, expected_after_delete, 2U) == EXIT_SUCCESS);

    EXPECT_TRUE(del_all_node_slist_while(&head));
    EXPECT_TRUE(head == NULL);
    return EXIT_SUCCESS;
}

static int test_dlist(void)
{
    DListNode *head = NULL;
    ListData expected_after_insert[] = {0, 1, 2, 3};
    ListData expected_after_update[] = {0, 1, 20, 3};
    ListData expected_after_delete[] = {1, 3};

    EXPECT_TRUE(creat_dlist(&head, 1));
    EXPECT_TRUE(head_add_dlist(&head, 0));
    EXPECT_TRUE(tail_add_dlist(&head, 3));
    EXPECT_TRUE(rand_add_dlist(&head, 1, 2));
    printf_dlist(head);
    EXPECT_TRUE(expect_dlist_sequence(head, expected_after_insert, 4U) == EXIT_SUCCESS);

    EXPECT_TRUE(find_node_dlist(head, 2) != NULL);
    EXPECT_TRUE(update_node_dlist(head, 2, 20));
    EXPECT_TRUE(expect_dlist_sequence(head, expected_after_update, 4U) == EXIT_SUCCESS);

    EXPECT_TRUE(del_node_dlist(&head, 20));
    EXPECT_TRUE(del_node_dlist(&head, 0));
    EXPECT_TRUE(expect_dlist_sequence(head, expected_after_delete, 2U) == EXIT_SUCCESS);

    EXPECT_TRUE(del_all_node_dlist(&head));
    EXPECT_TRUE(head == NULL);
    return EXIT_SUCCESS;
}

static int test_dlist_while(void)
{
    DListWhileNode *head = NULL;
    DListWhileNode *single = NULL;
    ListData expected_after_insert[] = {0, 1, 3, 2};
    ListData expected_after_update[] = {0, 1, 30, 2};
    ListData expected_after_delete[] = {1, 2};

    EXPECT_TRUE(creat_dlist_while(&single, 9));
    EXPECT_TRUE(tail_add_dlist_while(&single, 10));
    EXPECT_TRUE(single->next != single);
    EXPECT_TRUE(single->prev != single);
    EXPECT_TRUE(del_all_node_dlist_while(&single));
    EXPECT_TRUE(single == NULL);

    EXPECT_TRUE(creat_dlist_while(&head, 1));
    EXPECT_TRUE(tail_add_dlist_while(&head, 2));
    EXPECT_TRUE(head_add_dlist_while(&head, 0));
    EXPECT_TRUE(rand_add_dlist_while(&head, 1, 3));
    printf_dlist_while(head);
    EXPECT_TRUE(expect_dlist_while_sequence(head, expected_after_insert, 4U) == EXIT_SUCCESS);

    EXPECT_TRUE(find_node_dlist_while(head, 3) != NULL);
    EXPECT_TRUE(update_node_dlist_while(head, 3, 30));
    EXPECT_TRUE(expect_dlist_while_sequence(head, expected_after_update, 4U) == EXIT_SUCCESS);

    EXPECT_TRUE(del_node_dlist_while(&head, 30));
    EXPECT_TRUE(del_node_dlist_while(&head, 0));
    EXPECT_TRUE(expect_dlist_while_sequence(head, expected_after_delete, 2U) == EXIT_SUCCESS);

    EXPECT_TRUE(del_all_node_dlist_while(&head));
    EXPECT_TRUE(head == NULL);
    return EXIT_SUCCESS;
}

int main(void)
{
    if (test_slist() != EXIT_SUCCESS ||
        test_slist_while() != EXIT_SUCCESS ||
        test_dlist() != EXIT_SUCCESS ||
        test_dlist_while() != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    printf("list tests passed\n");
    return EXIT_SUCCESS;
}
