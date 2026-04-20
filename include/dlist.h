// 双向链表
#ifndef DLIST_H
#define DLIST_H

#include "slist.h"

// 双向链表结构体
typedef struct dlistnode
{
    ListData data;          // 数据域
    struct dlistnode *prev; // 指针域:指向上一个节点
    struct dlistnode *next; // 指针域:指向下一个节点
} DListNode;

/**
 * @brief 创建双向链表
 *
 * @param head 传入的头指针
 * @param data 初始数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool creat_dlist(DListNode **head, ListData data);

/**
 * @brief 头插法插入节点
 *
 * @param head 传入的头指针
 * @param data 要插入的数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool head_add_dlist(DListNode **head, ListData data);

/**
 * @brief 尾插法插入节点
 *
 * @param head 传入的头指针
 * @param data 要插入的数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool tail_add_dlist(DListNode **head, ListData data);

/**
 * @brief 指定位置插入数据
 *
 * @param head 传入的头指针
 * @param pos 指定位置
 * @param data 要插入的数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool rand_add_dlist(DListNode **head, ListData pos, ListData data);

/**
 * @brief 遍历链表(打印链表)
 *
 * @param head 传入的头指针
 * 特别提醒:如果你要使用自定义数据类型,请修改此函数!
 */
void printf_dlist(const DListNode *head);

/**
 * @brief 查找指定数据的节点
 *
 * @param head 待查询的链表
 * @param data 待查询的数据
 * @return 成功返回节点指针，失败返回NULL
 */
DListNode *find_node_dlist(const DListNode *head, ListData data);

/**
 * @brief 更新节点数据
 *
 * @param head 传入的头指针
 * @param old_data 待操作的节点数据
 * @param new_data 等待更新的目标数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool update_node_dlist(DListNode *head, ListData old_data, ListData new_data);

/**
 * @brief 删除指定数据的节点
 *
 * @param head 传入的头指针
 * @param data 要删除的数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool del_node_dlist(DListNode **head, ListData data);

/**
 * @brief 销毁链表
 *
 * @param head 传入的头指针
 * @return true 成功返回
 * @return false 失败返回
 */
bool del_all_node_dlist(DListNode **head);

#endif
