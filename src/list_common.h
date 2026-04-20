#ifndef LIST_COMMON_H
#define LIST_COMMON_H

#include <stdbool.h>
#include <stdlib.h>

#define LIST_DESTROY_LINEAR(list_ptr, node_type, start_member, next_member, destroy_member) \
    do                                                                                      \
    {                                                                                       \
        node_type *node = (list_ptr)->start_member;                                         \
        while (node != NULL)                                                                \
        {                                                                                   \
            node_type *next = node->next_member;                                            \
            if ((list_ptr)->destroy_member != NULL)                                         \
            {                                                                               \
                (list_ptr)->destroy_member(node->data);                                     \
            }                                                                               \
            free(node);                                                                     \
            node = next;                                                                    \
        }                                                                                   \
    } while (0)

#define LIST_DESTROY_COUNTED(list_ptr, node_type, start_member, next_member, size_member, destroy_member) \
    do                                                                                                      \
    {                                                                                                       \
        size_t remaining = (list_ptr)->size_member;                                                         \
        node_type *node = (list_ptr)->start_member;                                                         \
        while (remaining > 0U && node != NULL)                                                              \
        {                                                                                                   \
            node_type *next = node->next_member;                                                            \
            if ((list_ptr)->destroy_member != NULL)                                                         \
            {                                                                                               \
                (list_ptr)->destroy_member(node->data);                                                     \
            }                                                                                               \
            free(node);                                                                                     \
            node = next;                                                                                    \
            remaining--;                                                                                    \
        }                                                                                                   \
    } while (0)

#define LIST_TRAVERSE_LINEAR(list_ptr, node_type, start_member, next_member, visit, ctx) \
    do                                                                                    \
    {                                                                                     \
        node_type *node = (list_ptr)->start_member;                                       \
        while (node != NULL)                                                              \
        {                                                                                 \
            (visit)(node->data, (ctx));                                                   \
            node = node->next_member;                                                     \
        }                                                                                 \
    } while (0)

#define LIST_TRAVERSE_COUNTED(list_ptr, node_type, start_member, next_member, size_member, visit, ctx) \
    do                                                                                                  \
    {                                                                                                   \
        size_t index;                                                                                   \
        node_type *node = (list_ptr)->start_member;                                                     \
        for (index = 0U; index < (list_ptr)->size_member; ++index)                                      \
        {                                                                                               \
            (visit)(node->data, (ctx));                                                                 \
            node = node->next_member;                                                                   \
        }                                                                                               \
    } while (0)

static inline bool list_is_empty_size(size_t size) { return size == 0U; }

#endif
