#ifndef RUNTIME_STORE_H
#define RUNTIME_STORE_H

#include <stdbool.h>
#include <stddef.h>

#include "dlist.h"
#include "dlist_while.h"
#include "slist.h"
#include "slist_while.h"
#include "tree_bst.h"

typedef struct
{
    ListNode *slist;
    SListWhileNode *slist_while;
    DListNode *dlist;
    DListWhileNode *dlist_while;
    BSTNode *tree_bst;
} RuntimeState;

typedef struct
{
    size_t files_loaded;
    size_t files_saved;
    size_t files_missing;
    size_t values_loaded;
    size_t values_saved;
    size_t skipped_lines;
    bool had_error;
} RuntimeStateReport;

void runtime_state_init(RuntimeState *state);
void runtime_state_destroy(RuntimeState *state);
bool runtime_state_load(RuntimeState *state, RuntimeStateReport *report);
bool runtime_state_save(const RuntimeState *state, RuntimeStateReport *report);
bool runtime_state_save_with_seed(const RuntimeState *state,
                                  RuntimeStateReport *report,
                                  unsigned int seed);

#endif
