// 单向循环链表
#ifndef SLIST_WHILE_H
#define SLIST_WHILE_H

#include "slist.h"

// 单向循环链表结构体
typedef struct slistwhilenode
{
    ListData data;               // 数据域
    struct slistwhilenode *next; // 指针域:指向下一个节点
} SListWhileNode;

/**
 * @brief 创建单向循环链表
 *
 * @param head 传入的头指针
 * @param data 初始数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool creat_slist_while(SListWhileNode **head, ListData data);

/**
 * @brief 头插法插入节点
 *
 * @param head 传入的头指针
 * @param data 要插入的数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool head_add_slist_while(SListWhileNode **head, ListData data);

/**
 * @brief 尾插法插入节点
 *
 * @param head 传入的头指针
 * @param data 要插入的数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool tail_add_slist_while(SListWhileNode **head, ListData data);

/**
 * @brief 指定位置插入数据
 *
 * @param head 传入的头指针
 * @param pos 指定位置
 * @param data 要插入的数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool rand_add_slist_while(SListWhileNode **head, ListData pos, ListData data);

/**
 * @brief 遍历链表(打印链表)
 *
 * @param head 传入的头指针
 * 特别提醒:如果你要使用自定义数据类型,请修改此函数!
 */
void printf_slist_while(const SListWhileNode *head);

/**
 * @brief 查找指定数据的节点
 *
 * @param head 待查询的链表
 * @param data 待查询的数据
 * @return 成功返回节点指针，失败返回NULL
 */
SListWhileNode *find_node_slist_while(const SListWhileNode *head, ListData data);

/**
 * @brief 更新节点数据
 *
 * @param head 传入的头指针
 * @param old_data 待操作的节点数据
 * @param new_data 等待更新的目标数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool update_node_slist_while(SListWhileNode *head, ListData old_data, ListData new_data);

/**
 * @brief 删除指定数据的节点
 *
 * @param head 传入的头指针
 * @param data 要删除的数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool del_node_slist_while(SListWhileNode **head, ListData data);

/**
 * @brief 销毁链表
 *
 * @param head 传入的头指针
 * @return true 成功返回
 * @return false 失败返回
 */
bool del_all_node_slist_while(SListWhileNode **head);

#endif
