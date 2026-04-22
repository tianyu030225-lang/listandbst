#include "runtime_store.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define RUNTIME_DATA_DIR "runtime_data"
#define RUNTIME_PATH_SIZE 256U

typedef struct
{
    ListData *values;
    size_t count;
    size_t capacity;
} ValueBuffer;

static void runtime_report_reset(RuntimeStateReport *report)
{
    if (report != NULL)
    {
        memset(report, 0, sizeof(*report));
    }
}

static void mark_error(RuntimeStateReport *report)
{
    if (report != NULL)
    {
        report->had_error = true;
    }
}

static bool append_value(ValueBuffer *buffer, ListData value)
{
    ListData *new_values;
    size_t new_capacity;

    if (buffer == NULL)
    {
        return false;
    }

    if (buffer->count == buffer->capacity)
    {
        new_capacity = buffer->capacity == 0U ? 8U : buffer->capacity * 2U;
        new_values = (ListData *)realloc(buffer->values, new_capacity * sizeof(*new_values));
        if (new_values == NULL)
        {
            return false;
        }

        buffer->values = new_values;
        buffer->capacity = new_capacity;
    }

    buffer->values[buffer->count++] = value;
    return true;
}

static void free_buffer(ValueBuffer *buffer)
{
    if (buffer == NULL)
    {
        return;
    }

    free(buffer->values);
    buffer->values = NULL;
    buffer->count = 0U;
    buffer->capacity = 0U;
}

static void build_runtime_path(const char *filename, char *path, size_t size)
{
    (void)snprintf(path, size, "%s/%s", RUNTIME_DATA_DIR, filename);
}

static bool ensure_runtime_directory(RuntimeStateReport *report)
{
    struct stat st;

    if (stat(RUNTIME_DATA_DIR, &st) == 0)
    {
        if (!S_ISDIR(st.st_mode))
        {
            fprintf(stderr, "持久化:%s 已存在但不是目录!\n", RUNTIME_DATA_DIR);
            mark_error(report);
            return false;
        }

        return true;
    }

    if (errno != ENOENT)
    {
        fprintf(stderr, "持久化:无法检查 %s, %s\n", RUNTIME_DATA_DIR, strerror(errno));
        mark_error(report);
        return false;
    }

    if (mkdir(RUNTIME_DATA_DIR, 0755) != 0)
    {
        fprintf(stderr, "持久化:无法创建目录 %s, %s\n", RUNTIME_DATA_DIR, strerror(errno));
        mark_error(report);
        return false;
    }

    return true;
}

static bool load_values_from_file(const char *filename,
                                  ValueBuffer *buffer,
                                  RuntimeStateReport *report)
{
    char path[RUNTIME_PATH_SIZE];
    char line[128];
    FILE *stream;
    size_t line_number = 0U;

    build_runtime_path(filename, path, sizeof(path));
    stream = fopen(path, "r");
    if (stream == NULL)
    {
        if (errno == ENOENT)
        {
            if (report != NULL)
            {
                report->files_missing++;
            }
            return true;
        }

        fprintf(stderr, "持久化:无法读取 %s, %s\n", path, strerror(errno));
        mark_error(report);
        return false;
    }

    if (report != NULL)
    {
        report->files_loaded++;
    }

    while (fgets(line, sizeof(line), stream) != NULL)
    {
        char *cursor = line;
        char *endptr;
        long parsed;

        line_number++;
        while (isspace((unsigned char)*cursor))
        {
            cursor++;
        }

        if (*cursor == '\0')
        {
            continue;
        }

        errno = 0;
        parsed = strtol(cursor, &endptr, 10);
        while (isspace((unsigned char)*endptr))
        {
            endptr++;
        }

        if (cursor == endptr || errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX ||
            (*endptr != '\0' && *endptr != '\n'))
        {
            fprintf(stderr, "持久化:%s 第%zu行不是有效整数, 已跳过!\n", path, line_number);
            if (report != NULL)
            {
                report->skipped_lines++;
            }
            continue;
        }

        if (!append_value(buffer, (ListData)parsed))
        {
            fprintf(stderr, "持久化:加载 %s 时内存不足!\n", path);
            mark_error(report);
            fclose(stream);
            return false;
        }

        if (report != NULL)
        {
            report->values_loaded++;
        }
    }

    if (ferror(stream))
    {
        fprintf(stderr, "持久化:读取 %s 时发生 I/O 错误!\n", path);
        mark_error(report);
        fclose(stream);
        return false;
    }

    if (fclose(stream) != 0)
    {
        fprintf(stderr, "持久化:关闭 %s 失败, %s\n", path, strerror(errno));
        mark_error(report);
        return false;
    }

    return true;
}

static bool write_values_to_file(const char *filename,
                                 const ValueBuffer *buffer,
                                 RuntimeStateReport *report)
{
    char path[RUNTIME_PATH_SIZE];
    FILE *stream;
    size_t i;

    build_runtime_path(filename, path, sizeof(path));
    stream = fopen(path, "w");
    if (stream == NULL)
    {
        fprintf(stderr, "持久化:无法写入 %s, %s\n", path, strerror(errno));
        mark_error(report);
        return false;
    }

    for (i = 0U; i < buffer->count; ++i)
    {
        if (fprintf(stream, "%d\n", buffer->values[i]) < 0)
        {
            fprintf(stderr, "持久化:写入 %s 失败!\n", path);
            mark_error(report);
            fclose(stream);
            return false;
        }
    }

    if (fclose(stream) != 0)
    {
        fprintf(stderr, "持久化:关闭 %s 失败, %s\n", path, strerror(errno));
        mark_error(report);
        return false;
    }

    if (report != NULL)
    {
        report->files_saved++;
        report->values_saved += buffer->count;
    }

    return true;
}

static unsigned int next_shuffle_value(unsigned int *state)
{
    *state = (*state * 1103515245u) + 12345u;
    return *state;
}

static void shuffle_buffer(ValueBuffer *buffer, unsigned int *seed)
{
    size_t i;

    if (buffer == NULL || seed == NULL || buffer->count < 2U)
    {
        return;
    }

    for (i = buffer->count - 1U; i > 0U; --i)
    {
        size_t j = (size_t)(next_shuffle_value(seed) % (unsigned int)(i + 1U));
        ListData temp = buffer->values[i];
        buffer->values[i] = buffer->values[j];
        buffer->values[j] = temp;
    }
}

static bool collect_slist_values(const ListNode *head, ValueBuffer *buffer)
{
    const ListNode *node = head;

    while (node != NULL)
    {
        if (!append_value(buffer, node->data))
        {
            return false;
        }
        node = node->next;
    }

    return true;
}

static bool collect_slist_while_values(const SListWhileNode *head, ValueBuffer *buffer)
{
    const SListWhileNode *node = head;

    if (head == NULL)
    {
        return true;
    }

    do
    {
        if (!append_value(buffer, node->data))
        {
            return false;
        }
        node = node->next;
    } while (node != head);

    return true;
}

static bool collect_dlist_values(const DListNode *head, ValueBuffer *buffer)
{
    const DListNode *node = head;

    while (node != NULL)
    {
        if (!append_value(buffer, node->data))
        {
            return false;
        }
        node = node->next;
    }

    return true;
}

static bool collect_dlist_while_values(const DListWhileNode *head, ValueBuffer *buffer)
{
    const DListWhileNode *node = head;

    if (head == NULL)
    {
        return true;
    }

    do
    {
        if (!append_value(buffer, node->data))
        {
            return false;
        }
        node = node->next;
    } while (node != head);

    return true;
}

static bool collect_tree_values_preorder(const BSTNode *root, ValueBuffer *buffer)
{
    if (root == NULL)
    {
        return true;
    }

    if (!append_value(buffer, root->data))
    {
        return false;
    }

    if (!collect_tree_values_preorder(root->left, buffer))
    {
        return false;
    }

    return collect_tree_values_preorder(root->right, buffer);
}

static bool populate_slist(ListNode **head, const ValueBuffer *buffer)
{
    size_t i;

    for (i = 0U; i < buffer->count; ++i)
    {
        bool ok = *head == NULL ? creat_slist(head, buffer->values[i]) : tail_add_slist(head, buffer->values[i]);
        if (!ok)
        {
            return false;
        }
    }

    return true;
}

static bool populate_slist_while(SListWhileNode **head, const ValueBuffer *buffer)
{
    size_t i;

    for (i = 0U; i < buffer->count; ++i)
    {
        bool ok = *head == NULL ? creat_slist_while(head, buffer->values[i]) : tail_add_slist_while(head, buffer->values[i]);
        if (!ok)
        {
            return false;
        }
    }

    return true;
}

static bool populate_dlist(DListNode **head, const ValueBuffer *buffer)
{
    size_t i;

    for (i = 0U; i < buffer->count; ++i)
    {
        bool ok = *head == NULL ? creat_dlist(head, buffer->values[i]) : tail_add_dlist(head, buffer->values[i]);
        if (!ok)
        {
            return false;
        }
    }

    return true;
}

static bool populate_dlist_while(DListWhileNode **head, const ValueBuffer *buffer)
{
    size_t i;

    for (i = 0U; i < buffer->count; ++i)
    {
        bool ok = *head == NULL ? creat_dlist_while(head, buffer->values[i]) : tail_add_dlist_while(head, buffer->values[i]);
        if (!ok)
        {
            return false;
        }
    }

    return true;
}

static bool populate_tree(BSTNode **root, const ValueBuffer *buffer)
{
    size_t i;

    for (i = 0U; i < buffer->count; ++i)
    {
        bool ok = *root == NULL ? creat_tree_bst(root, buffer->values[i]) : add_node_tree_bst(root, buffer->values[i]);
        if (!ok)
        {
            return false;
        }
    }

    return true;
}

static bool load_structure(const char *filename,
                           RuntimeStateReport *report,
                           bool (*populate)(void *, const ValueBuffer *),
                           void *target)
{
    ValueBuffer buffer = {0};
    bool ok = load_values_from_file(filename, &buffer, report);

    if (ok)
    {
        ok = populate(target, &buffer);
        if (!ok)
        {
            fprintf(stderr, "持久化:从 %s 恢复结构失败!\n", filename);
            mark_error(report);
        }
    }

    free_buffer(&buffer);
    return ok;
}

static bool save_structure(const char *filename,
                           RuntimeStateReport *report,
                           unsigned int *seed,
                           bool (*collect)(const void *, ValueBuffer *),
                           const void *source)
{
    ValueBuffer buffer = {0};
    bool ok = collect(source, &buffer);

    if (!ok)
    {
        fprintf(stderr, "持久化:整理 %s 的内存数据时失败!\n", filename);
        mark_error(report);
        free_buffer(&buffer);
        return false;
    }

    shuffle_buffer(&buffer, seed);
    ok = write_values_to_file(filename, &buffer, report);
    free_buffer(&buffer);
    return ok;
}

void runtime_state_init(RuntimeState *state)
{
    if (state != NULL)
    {
        memset(state, 0, sizeof(*state));
    }
}

void runtime_state_destroy(RuntimeState *state)
{
    if (state == NULL)
    {
        return;
    }

    if (state->slist != NULL)
    {
        (void)del_all_node_slist(&state->slist);
    }
    if (state->slist_while != NULL)
    {
        (void)del_all_node_slist_while(&state->slist_while);
    }
    if (state->dlist != NULL)
    {
        (void)del_all_node_dlist(&state->dlist);
    }
    if (state->dlist_while != NULL)
    {
        (void)del_all_node_dlist_while(&state->dlist_while);
    }
    if (state->tree_bst != NULL)
    {
        (void)del_all_node_tree_bst(&state->tree_bst);
    }
}

bool runtime_state_load(RuntimeState *state, RuntimeStateReport *report)
{
    bool ok = true;

    if (state == NULL)
    {
        return false;
    }

    runtime_report_reset(report);
    runtime_state_destroy(state);
    runtime_state_init(state);

    ok = load_structure("slist.txt", report, (bool (*)(void *, const ValueBuffer *))populate_slist, &state->slist) && ok;
    ok = load_structure("slist_while.txt", report, (bool (*)(void *, const ValueBuffer *))populate_slist_while, &state->slist_while) && ok;
    ok = load_structure("dlist.txt", report, (bool (*)(void *, const ValueBuffer *))populate_dlist, &state->dlist) && ok;
    ok = load_structure("dlist_while.txt", report, (bool (*)(void *, const ValueBuffer *))populate_dlist_while, &state->dlist_while) && ok;
    ok = load_structure("tree_bst.txt", report, (bool (*)(void *, const ValueBuffer *))populate_tree, &state->tree_bst) && ok;
    return ok;
}

bool runtime_state_save(const RuntimeState *state, RuntimeStateReport *report)
{
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)getpid();
    return runtime_state_save_with_seed(state, report, seed);
}

bool runtime_state_save_with_seed(const RuntimeState *state,
                                  RuntimeStateReport *report,
                                  unsigned int seed)
{
    bool ok = true;

    if (state == NULL)
    {
        return false;
    }

    runtime_report_reset(report);
    if (!ensure_runtime_directory(report))
    {
        return false;
    }

    ok = save_structure("slist.txt", report, &seed, (bool (*)(const void *, ValueBuffer *))collect_slist_values, state->slist) && ok;
    ok = save_structure("slist_while.txt", report, &seed, (bool (*)(const void *, ValueBuffer *))collect_slist_while_values, state->slist_while) && ok;
    ok = save_structure("dlist.txt", report, &seed, (bool (*)(const void *, ValueBuffer *))collect_dlist_values, state->dlist) && ok;
    ok = save_structure("dlist_while.txt", report, &seed, (bool (*)(const void *, ValueBuffer *))collect_dlist_while_values, state->dlist_while) && ok;
    ok = save_structure("tree_bst.txt", report, &seed, (bool (*)(const void *, ValueBuffer *))collect_tree_values_preorder, state->tree_bst) && ok;
    return ok;
}
