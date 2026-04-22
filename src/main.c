#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "api_catalog.h"
#include "listandtree.h"
#include "runtime_store.h"
#include "show.h"

#define STRUCTURE_COUNT 5U
#define DEMO_PAGE_COUNT 10U
#define TEST_GROUP_COUNT 3U

static void show_messagef(UiTone tone, const char *format, ...)
{
    char buffer[512];
    va_list args;

    va_start(args, format);
    (void)vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    ui_show_message(stdout, tone, buffer);
}

static void show_load_report(const RuntimeStateReport *report)
{
    if (report == NULL)
    {
        return;
    }

    if (report->had_error)
    {
        show_messagef(UI_TONE_WARNING,
                      "本地数据加载完成，但存在异常: 已载入 %zu 条数据，跳过 %zu 行。",
                      report->values_loaded,
                      report->skipped_lines);
        return;
    }

    if (report->values_loaded == 0U && report->files_missing == 5U)
    {
        ui_show_message(stdout, UI_TONE_INFO, "未发现历史数据，本次从空结构开始。");
        return;
    }

    show_messagef(UI_TONE_SUCCESS,
                  "本地数据已恢复: 读取 %zu 个文件，共载入 %zu 条数据。",
                  report->files_loaded,
                  report->values_loaded);
    if (report->skipped_lines > 0U)
    {
        show_messagef(UI_TONE_WARNING, "持久化文件里有 %zu 行无效数据已被跳过。", report->skipped_lines);
    }
}

static void show_save_report(const RuntimeStateReport *report)
{
    if (report == NULL)
    {
        return;
    }

    if (report->had_error)
    {
        show_messagef(UI_TONE_ERROR,
                      "退出前保存失败: 已尝试写入 %zu 个文件，成功保存 %zu 条数据。",
                      report->files_saved,
                      report->values_saved);
        return;
    }

    show_messagef(UI_TONE_SUCCESS,
                  "退出前已保存本地数据: 写入 %zu 个文件，共 %zu 条数据。",
                  report->files_saved,
                  report->values_saved);
}

static bool read_line(char *buffer, size_t size)
{
    if (fgets(buffer, (int)size, stdin) == NULL)
    {
        return false;
    }

    buffer[strcspn(buffer, "\n")] = '\0';
    return true;
}

static bool prompt_int(const char *prompt, int *value)
{
    char buffer[64];
    char *endptr;
    long parsed;

    if (prompt == NULL || value == NULL)
    {
        return false;
    }

    ui_prompt(stdout, prompt);
    if (!read_line(buffer, sizeof(buffer)))
    {
        return false;
    }

    parsed = strtol(buffer, &endptr, 10);
    if (buffer[0] == '\0' || *endptr != '\0')
    {
        return false;
    }

    *value = (int)parsed;
    return true;
}

static bool prompt_two_ints(const char *first_prompt,
                            int *first_value,
                            const char *second_prompt,
                            int *second_value)
{
    return prompt_int(first_prompt, first_value) &&
           prompt_int(second_prompt, second_value);
}

static void page_linef(const char *format, ...)
{
    char buffer[512];
    va_list args;

    va_start(args, format);
    (void)vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    ui_streamf(stdout, UI_STREAM_QUICK, " %s\n", buffer);
}

static void wait_for_operation(void)
{
    ui_wait_for_enter(stdout, "按回车继续操作...");
}

static size_t count_readable_paths(const char *const *paths, size_t count)
{
    size_t ready = 0U;
    size_t i;

    for (i = 0U; i < count; ++i)
    {
        if (access(paths[i], R_OK) == 0)
        {
            ready++;
        }
    }

    return ready;
}

static const ApiGroupInfo *find_group_info(ApiGroupId group_id)
{
    const ApiGroupInfo *groups;
    size_t count;
    size_t i;

    groups = api_catalog_groups(&count);
    for (i = 0U; i < count; ++i)
    {
        if (groups[i].id == group_id)
        {
            return &groups[i];
        }
    }

    return NULL;
}

static void render_group_api(ApiGroupId group_id)
{
    const ApiGroupInfo *group = find_group_info(group_id);
    const ApiEntry *entries;
    size_t entry_count;
    size_t i;

    entries = api_catalog_entries(&entry_count);
    page_linef("API 一览%s%s",
               group != NULL ? " - " : "",
               group != NULL ? group->name : "未知分组");
    for (i = 0U; i < entry_count; ++i)
    {
        if (entries[i].group_id == group_id)
        {
            page_linef("  - %s: %s", entries[i].name, entries[i].summary);
        }
    }
    page_linef("");
}

static void run_bootstrap_sequence(void)
{
    static const char *const PUBLIC_HEADERS[] = {
        "include/slist.h",
        "include/slist_while.h",
        "include/dlist.h",
        "include/dlist_while.h",
        "include/tree_bst.h",
        "include/paixu_search.h",
        "include/listandtree.h"};
    static const char *const SUPPORT_FILES[] = {
        "tests/test_lists.c",
        "tests/test_bst.c",
        "tests/test_algorithms.c",
        "tests/test_runtime_store.c",
        "share/user_makefile",
        "tools/build_share_assets.py",
        "tests/test_share_export.sh"};
    size_t header_ready;
    size_t support_ready;
    bool bootstrap_ready;

    header_ready = count_readable_paths(PUBLIC_HEADERS, sizeof(PUBLIC_HEADERS) / sizeof(PUBLIC_HEADERS[0]));
    support_ready = count_readable_paths(SUPPORT_FILES, sizeof(SUPPORT_FILES) / sizeof(SUPPORT_FILES[0]));
    bootstrap_ready = api_catalog_total_count() > 0U &&
                      header_ready == (sizeof(PUBLIC_HEADERS) / sizeof(PUBLIC_HEADERS[0])) &&
                      support_ready == (sizeof(SUPPORT_FILES) / sizeof(SUPPORT_FILES[0]));
    (void)bootstrap_ready;

    ui_show_welcome();
}

static void show_slist_demo(RuntimeState *state)
{
    ListNode *head;
    int choice;

    if (state == NULL)
    {
        return;
    }

    head = state->slist;

    while (true)
    {
        ui_show_section(stdout,
                        NULL,
                        "单向链表 SList",
                        "由你手动执行 create、head_add、tail_add、rand_add、find、update、delete。");

        if (head == NULL)
        {
            page_linef("当前链表为空。");
        }
        else
        {
            page_linef("当前链表:");
            printf_slist(head);
        }
        page_linef("");
        page_linef("[1] 创建首节点");
        page_linef("[2] 头插节点");
        page_linef("[3] 尾插节点");
        page_linef("[4] 在指定数据后插入");
        page_linef("[5] 打印链表");
        page_linef("[6] 查找节点");
        page_linef("[7] 更新节点数据");
        page_linef("[8] 删除指定数据");
        page_linef("[9] 销毁链表");
        page_linef("[0] 返回主菜单");

        if (!prompt_int("请选择操作: ", &choice))
        {
            ui_show_message(stdout, UI_TONE_ERROR, "输入无效，请重新输入整数。");
            wait_for_operation();
            continue;
        }

        if (choice == 0)
        {
            break;
        }

        switch (choice)
        {
        case 1:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入首节点数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "创建失败，输入无效。");
                break;
            }
            ok = creat_slist(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "首节点已创建。" : "creat_slist 执行失败。");
            break;
        }
        case 2:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入头插数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "头插失败，输入无效。");
                break;
            }
            ok = head_add_slist(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "头插完成。" : "head_add_slist 执行失败。");
            break;
        }
        case 3:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入尾插数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "尾插失败，输入无效。");
                break;
            }
            ok = tail_add_slist(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "尾插完成。" : "tail_add_slist 执行失败。");
            break;
        }
        case 4:
        {
            int pos;
            int data;
            bool ok;

            if (!prompt_two_ints("请输入目标节点数据: ", &pos, "请输入新节点数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "插入失败，输入无效。");
                break;
            }
            ok = rand_add_slist(&head, pos, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "指定数据后插入完成。" : "rand_add_slist 执行失败。");
            break;
        }
        case 5:
            if (head == NULL)
            {
                ui_show_message(stdout, UI_TONE_WARNING, "链表为空，暂无可打印内容。");
            }
            else
            {
                printf_slist(head);
            }
            break;
        case 6:
        {
            int data;
            ListNode *node;

            if (!prompt_int("请输入要查找的数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "查找失败，输入无效。");
                break;
            }
            node = find_node_slist(head, data);
            show_messagef(node != NULL ? UI_TONE_SUCCESS : UI_TONE_WARNING,
                          "find_node_slist(%d) -> %s",
                          data,
                          node != NULL ? "命中" : "未命中");
            break;
        }
        case 7:
        {
            int old_data;
            int new_data;
            bool ok;

            if (!prompt_two_ints("请输入原数据: ", &old_data, "请输入新数据: ", &new_data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "更新失败，输入无效。");
                break;
            }
            ok = update_node_slist(head, old_data, new_data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "节点数据已更新。" : "update_node_slist 执行失败。");
            break;
        }
        case 8:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入要删除的数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "删除失败，输入无效。");
                break;
            }
            ok = del_node_slist(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "节点已删除。" : "del_node_slist 执行失败。");
            break;
        }
        case 9:
            if (head == NULL)
            {
                ui_show_message(stdout, UI_TONE_WARNING, "链表已经为空。");
            }
            else
            {
                (void)del_all_node_slist(&head);
                ui_show_message(stdout, UI_TONE_SUCCESS, "链表已销毁。");
            }
            break;
        default:
            ui_show_message(stdout, UI_TONE_WARNING, "无效操作，请重试。");
            break;
        }

        wait_for_operation();
    }

    state->slist = head;
}

static void show_slist_while_demo(RuntimeState *state)
{
    SListWhileNode *head;
    int choice;

    if (state == NULL)
    {
        return;
    }

    head = state->slist_while;

    while (true)
    {
        ui_show_section(stdout,
                        NULL,
                        "单向循环链表 SListWhile",
                        "由你手动执行 create、head_add、tail_add、rand_add、find、update、delete。");

        if (head == NULL)
        {
            page_linef("当前循环链表为空。");
        }
        else
        {
            page_linef("当前循环链表:");
            printf_slist_while(head);
        }
        page_linef("");
        page_linef("[1] 创建首节点");
        page_linef("[2] 头插节点");
        page_linef("[3] 尾插节点");
        page_linef("[4] 在指定数据后插入");
        page_linef("[5] 打印链表");
        page_linef("[6] 查找节点");
        page_linef("[7] 更新节点数据");
        page_linef("[8] 删除指定数据");
        page_linef("[9] 销毁链表");
        page_linef("[0] 返回主菜单");

        if (!prompt_int("请选择操作: ", &choice))
        {
            ui_show_message(stdout, UI_TONE_ERROR, "输入无效，请重新输入整数。");
            wait_for_operation();
            continue;
        }

        if (choice == 0)
        {
            break;
        }

        switch (choice)
        {
        case 1:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入首节点数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "创建失败，输入无效。");
                break;
            }
            ok = creat_slist_while(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "首节点已创建。" : "creat_slist_while 执行失败。");
            break;
        }
        case 2:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入头插数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "头插失败，输入无效。");
                break;
            }
            ok = head_add_slist_while(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "头插完成。" : "head_add_slist_while 执行失败。");
            break;
        }
        case 3:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入尾插数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "尾插失败，输入无效。");
                break;
            }
            ok = tail_add_slist_while(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "尾插完成。" : "tail_add_slist_while 执行失败。");
            break;
        }
        case 4:
        {
            int pos;
            int data;
            bool ok;

            if (!prompt_two_ints("请输入目标节点数据: ", &pos, "请输入新节点数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "插入失败，输入无效。");
                break;
            }
            ok = rand_add_slist_while(&head, pos, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "指定数据后插入完成。" : "rand_add_slist_while 执行失败。");
            break;
        }
        case 5:
            if (head == NULL)
            {
                ui_show_message(stdout, UI_TONE_WARNING, "链表为空，暂无可打印内容。");
            }
            else
            {
                printf_slist_while(head);
            }
            break;
        case 6:
        {
            int data;
            SListWhileNode *node;

            if (!prompt_int("请输入要查找的数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "查找失败，输入无效。");
                break;
            }
            node = find_node_slist_while(head, data);
            show_messagef(node != NULL ? UI_TONE_SUCCESS : UI_TONE_WARNING,
                          "find_node_slist_while(%d) -> %s",
                          data,
                          node != NULL ? "命中" : "未命中");
            break;
        }
        case 7:
        {
            int old_data;
            int new_data;
            bool ok;

            if (!prompt_two_ints("请输入原数据: ", &old_data, "请输入新数据: ", &new_data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "更新失败，输入无效。");
                break;
            }
            ok = update_node_slist_while(head, old_data, new_data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "节点数据已更新。" : "update_node_slist_while 执行失败。");
            break;
        }
        case 8:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入要删除的数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "删除失败，输入无效。");
                break;
            }
            ok = del_node_slist_while(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "节点已删除。" : "del_node_slist_while 执行失败。");
            break;
        }
        case 9:
            if (head == NULL)
            {
                ui_show_message(stdout, UI_TONE_WARNING, "链表已经为空。");
            }
            else
            {
                (void)del_all_node_slist_while(&head);
                ui_show_message(stdout, UI_TONE_SUCCESS, "链表已销毁。");
            }
            break;
        default:
            ui_show_message(stdout, UI_TONE_WARNING, "无效操作，请重试。");
            break;
        }

        wait_for_operation();
    }

    state->slist_while = head;
}

static void show_dlist_demo(RuntimeState *state)
{
    DListNode *head;
    int choice;

    if (state == NULL)
    {
        return;
    }

    head = state->dlist;

    while (true)
    {
        ui_show_section(stdout,
                        NULL,
                        "双向链表 DList",
                        "由你手动执行 create、head_add、tail_add、rand_add、find、update、delete。");

        if (head == NULL)
        {
            page_linef("当前双向链表为空。");
        }
        else
        {
            page_linef("当前双向链表:");
            printf_dlist(head);
        }
        page_linef("");
        page_linef("[1] 创建首节点");
        page_linef("[2] 头插节点");
        page_linef("[3] 尾插节点");
        page_linef("[4] 在指定数据后插入");
        page_linef("[5] 打印链表");
        page_linef("[6] 查找节点");
        page_linef("[7] 更新节点数据");
        page_linef("[8] 删除指定数据");
        page_linef("[9] 销毁链表");
        page_linef("[0] 返回主菜单");

        if (!prompt_int("请选择操作: ", &choice))
        {
            ui_show_message(stdout, UI_TONE_ERROR, "输入无效，请重新输入整数。");
            wait_for_operation();
            continue;
        }

        if (choice == 0)
        {
            break;
        }

        switch (choice)
        {
        case 1:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入首节点数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "创建失败，输入无效。");
                break;
            }
            ok = creat_dlist(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "首节点已创建。" : "creat_dlist 执行失败。");
            break;
        }
        case 2:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入头插数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "头插失败，输入无效。");
                break;
            }
            ok = head_add_dlist(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "头插完成。" : "head_add_dlist 执行失败。");
            break;
        }
        case 3:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入尾插数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "尾插失败，输入无效。");
                break;
            }
            ok = tail_add_dlist(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "尾插完成。" : "tail_add_dlist 执行失败。");
            break;
        }
        case 4:
        {
            int pos;
            int data;
            bool ok;

            if (!prompt_two_ints("请输入目标节点数据: ", &pos, "请输入新节点数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "插入失败，输入无效。");
                break;
            }
            ok = rand_add_dlist(&head, pos, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "指定数据后插入完成。" : "rand_add_dlist 执行失败。");
            break;
        }
        case 5:
            if (head == NULL)
            {
                ui_show_message(stdout, UI_TONE_WARNING, "链表为空，暂无可打印内容。");
            }
            else
            {
                printf_dlist(head);
            }
            break;
        case 6:
        {
            int data;
            DListNode *node;

            if (!prompt_int("请输入要查找的数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "查找失败，输入无效。");
                break;
            }
            node = find_node_dlist(head, data);
            show_messagef(node != NULL ? UI_TONE_SUCCESS : UI_TONE_WARNING,
                          "find_node_dlist(%d) -> %s",
                          data,
                          node != NULL ? "命中" : "未命中");
            break;
        }
        case 7:
        {
            int old_data;
            int new_data;
            bool ok;

            if (!prompt_two_ints("请输入原数据: ", &old_data, "请输入新数据: ", &new_data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "更新失败，输入无效。");
                break;
            }
            ok = update_node_dlist(head, old_data, new_data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "节点数据已更新。" : "update_node_dlist 执行失败。");
            break;
        }
        case 8:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入要删除的数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "删除失败，输入无效。");
                break;
            }
            ok = del_node_dlist(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "节点已删除。" : "del_node_dlist 执行失败。");
            break;
        }
        case 9:
            if (head == NULL)
            {
                ui_show_message(stdout, UI_TONE_WARNING, "链表已经为空。");
            }
            else
            {
                (void)del_all_node_dlist(&head);
                ui_show_message(stdout, UI_TONE_SUCCESS, "链表已销毁。");
            }
            break;
        default:
            ui_show_message(stdout, UI_TONE_WARNING, "无效操作，请重试。");
            break;
        }

        wait_for_operation();
    }

    state->dlist = head;
}

static void show_dlist_while_demo(RuntimeState *state)
{
    DListWhileNode *head;
    int choice;

    if (state == NULL)
    {
        return;
    }

    head = state->dlist_while;

    while (true)
    {
        ui_show_section(stdout,
                        NULL,
                        "双向循环链表 DListWhile",
                        "由你手动执行 create、head_add、tail_add、rand_add、find、update、delete。");

        if (head == NULL)
        {
            page_linef("当前双向循环链表为空。");
        }
        else
        {
            page_linef("当前双向循环链表:");
            printf_dlist_while(head);
        }
        page_linef("");
        page_linef("[1] 创建首节点");
        page_linef("[2] 头插节点");
        page_linef("[3] 尾插节点");
        page_linef("[4] 在指定数据后插入");
        page_linef("[5] 打印链表");
        page_linef("[6] 查找节点");
        page_linef("[7] 更新节点数据");
        page_linef("[8] 删除指定数据");
        page_linef("[9] 销毁链表");
        page_linef("[0] 返回主菜单");

        if (!prompt_int("请选择操作: ", &choice))
        {
            ui_show_message(stdout, UI_TONE_ERROR, "输入无效，请重新输入整数。");
            wait_for_operation();
            continue;
        }

        if (choice == 0)
        {
            break;
        }

        switch (choice)
        {
        case 1:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入首节点数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "创建失败，输入无效。");
                break;
            }
            ok = creat_dlist_while(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "首节点已创建。" : "creat_dlist_while 执行失败。");
            break;
        }
        case 2:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入头插数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "头插失败，输入无效。");
                break;
            }
            ok = head_add_dlist_while(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "头插完成。" : "head_add_dlist_while 执行失败。");
            break;
        }
        case 3:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入尾插数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "尾插失败，输入无效。");
                break;
            }
            ok = tail_add_dlist_while(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "尾插完成。" : "tail_add_dlist_while 执行失败。");
            break;
        }
        case 4:
        {
            int pos;
            int data;
            bool ok;

            if (!prompt_two_ints("请输入目标节点数据: ", &pos, "请输入新节点数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "插入失败，输入无效。");
                break;
            }
            ok = rand_add_dlist_while(&head, pos, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "指定数据后插入完成。" : "rand_add_dlist_while 执行失败。");
            break;
        }
        case 5:
            if (head == NULL)
            {
                ui_show_message(stdout, UI_TONE_WARNING, "链表为空，暂无可打印内容。");
            }
            else
            {
                printf_dlist_while(head);
            }
            break;
        case 6:
        {
            int data;
            DListWhileNode *node;

            if (!prompt_int("请输入要查找的数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "查找失败，输入无效。");
                break;
            }
            node = find_node_dlist_while(head, data);
            show_messagef(node != NULL ? UI_TONE_SUCCESS : UI_TONE_WARNING,
                          "find_node_dlist_while(%d) -> %s",
                          data,
                          node != NULL ? "命中" : "未命中");
            break;
        }
        case 7:
        {
            int old_data;
            int new_data;
            bool ok;

            if (!prompt_two_ints("请输入原数据: ", &old_data, "请输入新数据: ", &new_data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "更新失败，输入无效。");
                break;
            }
            ok = update_node_dlist_while(head, old_data, new_data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "节点数据已更新。" : "update_node_dlist_while 执行失败。");
            break;
        }
        case 8:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入要删除的数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "删除失败，输入无效。");
                break;
            }
            ok = del_node_dlist_while(&head, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "节点已删除。" : "del_node_dlist_while 执行失败。");
            break;
        }
        case 9:
            if (head == NULL)
            {
                ui_show_message(stdout, UI_TONE_WARNING, "链表已经为空。");
            }
            else
            {
                (void)del_all_node_dlist_while(&head);
                ui_show_message(stdout, UI_TONE_SUCCESS, "链表已销毁。");
            }
            break;
        default:
            ui_show_message(stdout, UI_TONE_WARNING, "无效操作，请重试。");
            break;
        }

        wait_for_operation();
    }

    state->dlist_while = head;
}

static void show_tree_bst_demo(RuntimeState *state)
{
    BSTNode *root;
    int choice;

    if (state == NULL)
    {
        return;
    }

    root = state->tree_bst;

    while (true)
    {
        ui_show_section(stdout,
                        NULL,
                        "二叉搜索树 TreeBST",
                        "由你手动执行 create、add、find、三种遍历、delete、destroy。");

        if (root == NULL)
        {
            page_linef("当前 BST 为空。");
        }
        else
        {
            page_linef("当前 BST 中序遍历:");
            printf_tree_bst_mid(root);
        }
        page_linef("");
        page_linef("[1] 创建根节点");
        page_linef("[2] 插入节点");
        page_linef("[3] 查找节点");
        page_linef("[4] 中序遍历");
        page_linef("[5] 前序遍历");
        page_linef("[6] 后序遍历");
        page_linef("[7] 删除指定数据");
        page_linef("[8] 销毁整棵树");
        page_linef("[0] 返回主菜单");

        if (!prompt_int("请选择操作: ", &choice))
        {
            ui_show_message(stdout, UI_TONE_ERROR, "输入无效，请重新输入整数。");
            wait_for_operation();
            continue;
        }

        if (choice == 0)
        {
            break;
        }

        switch (choice)
        {
        case 1:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入根节点数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "创建失败，输入无效。");
                break;
            }
            ok = creat_tree_bst(&root, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "根节点已创建。" : "creat_tree_bst 执行失败。");
            break;
        }
        case 2:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入插入数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "插入失败，输入无效。");
                break;
            }
            ok = add_node_tree_bst(&root, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "节点已插入。" : "add_node_tree_bst 执行失败。");
            break;
        }
        case 3:
        {
            int data;
            BSTNode *node;

            if (!prompt_int("请输入要查找的数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "查找失败，输入无效。");
                break;
            }
            node = find_node_tree_bst(root, data);
            show_messagef(node != NULL ? UI_TONE_SUCCESS : UI_TONE_WARNING,
                          "find_node_tree_bst(%d) -> %s",
                          data,
                          node != NULL ? "命中" : "未命中");
            break;
        }
        case 4:
            if (root == NULL)
            {
                ui_show_message(stdout, UI_TONE_WARNING, "BST 为空，暂无可打印内容。");
            }
            else
            {
                printf_tree_bst_mid(root);
            }
            break;
        case 5:
            if (root == NULL)
            {
                ui_show_message(stdout, UI_TONE_WARNING, "BST 为空，暂无可打印内容。");
            }
            else
            {
                printf_tree_bst_front(root);
            }
            break;
        case 6:
            if (root == NULL)
            {
                ui_show_message(stdout, UI_TONE_WARNING, "BST 为空，暂无可打印内容。");
            }
            else
            {
                printf_tree_bst_back(root);
            }
            break;
        case 7:
        {
            int data;
            bool ok;

            if (!prompt_int("请输入要删除的数据: ", &data))
            {
                ui_show_message(stdout, UI_TONE_ERROR, "删除失败，输入无效。");
                break;
            }
            ok = del_node_tree_bst(&root, data);
            ui_show_message(stdout, ok ? UI_TONE_SUCCESS : UI_TONE_ERROR, ok ? "节点已删除。" : "del_node_tree_bst 执行失败。");
            break;
        }
        case 8:
            if (root == NULL)
            {
                ui_show_message(stdout, UI_TONE_WARNING, "BST 已经为空。");
            }
            else
            {
                (void)del_all_node_tree_bst(&root);
                ui_show_message(stdout, UI_TONE_SUCCESS, "BST 已销毁。");
            }
            break;
        default:
            ui_show_message(stdout, UI_TONE_WARNING, "无效操作，请重试。");
            break;
        }

        wait_for_operation();
    }

    state->tree_bst = root;
}

static void show_paixu_search_demo(void)
{
    ListData bubble_values[] = {7, 2, 9, 4, 1, 5};
    ListData quick_values[] = {10, 8, 3, 6, 2, 9, 1};
    ListData sorted_values[] = {1, 2, 4, 5, 7, 9};
    int bubble_len = (int)(sizeof(bubble_values) / sizeof(bubble_values[0]));
    int quick_len = (int)(sizeof(quick_values) / sizeof(quick_values[0]));
    int sorted_len = (int)(sizeof(sorted_values) / sizeof(sorted_values[0]));

    ui_show_section(stdout,
                    NULL,
                    "排序与查找 PaiXuSearch",
                    "回切为原始数组 API：bubble_sort / quick_sort / binary_search / printf_arr。");

    render_group_api(API_GROUP_PAIXU_SEARCH);

    page_linef("bubble_sort 前:");
    printf_arr(bubble_values, bubble_len);
    (void)bubble_sort(bubble_values, bubble_len);
    page_linef("bubble_sort 后:");
    printf_arr(bubble_values, bubble_len);

    page_linef("quick_sort 前:");
    printf_arr(quick_values, quick_len);
    (void)quick_sort(quick_values, 0, quick_len - 1);
    page_linef("quick_sort 后:");
    printf_arr(quick_values, quick_len);

    page_linef("binary_search(sorted_values, %d, 5) -> %d",
               sorted_len,
               binary_search(sorted_values, sorted_len, 5));
    page_linef("binary_search(sorted_values, %d, 8) -> %d",
               sorted_len,
               binary_search(sorted_values, sorted_len, 8));

    ui_show_message(stdout, UI_TONE_SUCCESS, "PaiXuSearch 演示完成。");
}

static void show_api_overview(void)
{
    const ApiGroupInfo *groups;
    size_t group_count;
    size_t i;

    ui_show_section(stdout,
                    NULL,
                    "LIST and BST API 总览",
                    "当前版本已经回切为你原始的 48 个函数接口。");

    page_linef("统一总头文件");
    page_linef("  - include/listandtree.h");
    page_linef("");

    groups = api_catalog_groups(&group_count);
    for (i = 0U; i < group_count; ++i)
    {
        page_linef("%s (%zu)", groups[i].name, api_catalog_count_for_group(groups[i].id));
        page_linef("  - %s", groups[i].subtitle);
        render_group_api(groups[i].id);
    }

    show_messagef(UI_TONE_SUCCESS, "公开 API 总数: %zu", api_catalog_total_count());
}

static void show_structure_overview(void)
{
    ui_show_section(stdout,
                    NULL,
                    "结构对比",
                    "四种链表、BST 和排序模块都回切为节点/数组直连的原始调用方式。");

    page_linef("SList      : creat/head_add/tail_add/rand_add/find/update/delete/destroy。");
    page_linef("SListWhile : 使用 head 表示当前循环起点，按数据值定位插入与删除。");
    page_linef("DList      : 节点包含 prev/next，按数据值定位插入与删除。");
    page_linef("DListWhile : 双向循环节点同时维护 prev/next，支持单节点和多节点环。");
    page_linef("TreeBST    : 直接操作 root 指针，保留前/中/后序打印。");
    page_linef("PaiXuSearch: 直接面向整型数组和左右边界。");
    page_linef("");
    ui_show_message(stdout, UI_TONE_INFO, "当前版本不再展示容器对象式 API。");
}

static void show_build_and_test(void)
{
    ui_show_section(stdout,
                    NULL,
                    "构建与测试",
                    "当前工程继续保留开发版实验室和分享版导出器两条产物线。");

    page_linef("开发版命令");
    page_linef("  make");
    page_linef("  make run");
    page_linef("  make test");
    page_linef("  make share-check");
    page_linef("  make valgrind");
    page_linef("  make clean");
    page_linef("");
    page_linef("分享版命令");
    page_linef("  make share");
    page_linef("");
    page_linef("测试入口");
    page_linef("  - tests/test_lists.c");
    page_linef("  - tests/test_bst.c");
    page_linef("  - tests/test_algorithms.c");
    page_linef("  - tests/test_runtime_store.c");
    page_linef("");
    page_linef("分享版输出");
    page_linef("  - dist/listandtree.out");
    page_linef("  - 运行后在当前目录生成 listandtree/、README.md 和根 Makefile");
    page_linef("  - listandtree/ 内只包含公开头文件和 liblistandtree.a");
}

static void show_project_layout(void)
{
    ui_show_section(stdout,
                    NULL,
                    "项目结构",
                    "当前版本重点是原始 API 回切、分阶段快启，以及可分享导出。");

    page_linef("开发版目录");
    page_linef("  - include/: slist.h、slist_while.h、dlist.h、dlist_while.h、tree_bst.h、paixu_search.h、listandtree.h、show.h");
    page_linef("  - src/: main.c、share_main.c、show.c、runtime_store、四种链表实现、tree_bst.c、paixu_search.c、api_catalog");
    page_linef("  - tests/: list、BST、算法、持久化与分享导出验证");
    page_linef("");
    page_linef("分享版支撑文件");
    page_linef("  - share/user_makefile");
    page_linef("  - tools/build_share_assets.py");
    page_linef("  - dist/listandtree.out");
    page_linef("");
    page_linef("导出目录继续保持平铺，便于用户直接 include 并链接预编译静态库。");
}

static int run_list_lab(void)
{
    RuntimeState state;
    RuntimeStateReport report;
    int choice;
    int exit_code = EXIT_SUCCESS;

    runtime_state_init(&state);
    run_bootstrap_sequence();
    (void)runtime_state_load(&state, &report);
    show_load_report(&report);

    while (true)
    {
        bool pause_after_page = true;

        ui_show_main_menu(STRUCTURE_COUNT,
                          api_catalog_total_count(),
                          DEMO_PAGE_COUNT,
                          TEST_GROUP_COUNT);
        if (!prompt_int("请选择功能: ", &choice))
        {
            ui_show_message(stdout, UI_TONE_ERROR, "输入无效，请重新输入。");
            ui_wait_for_enter(stdout, "按回车返回主菜单...");
            continue;
        }

        if (choice == 0)
        {
            if (!runtime_state_save(&state, &report))
            {
                exit_code = EXIT_FAILURE;
            }
            show_save_report(&report);
            runtime_state_destroy(&state);
            ui_show_shutdown();
            return exit_code;
        }

        switch (choice)
        {
        case 1:
            show_slist_demo(&state);
            pause_after_page = false;
            break;
        case 2:
            show_slist_while_demo(&state);
            pause_after_page = false;
            break;
        case 3:
            show_dlist_demo(&state);
            pause_after_page = false;
            break;
        case 4:
            show_dlist_while_demo(&state);
            pause_after_page = false;
            break;
        case 5:
            show_tree_bst_demo(&state);
            pause_after_page = false;
            break;
        case 6:
            show_paixu_search_demo();
            break;
        case 7:
            show_api_overview();
            break;
        case 8:
            show_structure_overview();
            break;
        case 9:
            show_build_and_test();
            break;
        case 10:
            show_project_layout();
            break;
        default:
            ui_show_message(stdout, UI_TONE_WARNING, "无效选项，请重试。");
            break;
        }

        if (pause_after_page)
        {
            ui_wait_for_enter(stdout, "按回车返回主菜单...");
        }
    }
}

int main(void)
{
    return run_list_lab();
}
