#include "dlist.h"
#include "listandtree_export.h"

// 创建新节点
static DListNode *__creat_newnode(ListData data)
{
    DListNode *newnode = (DListNode *)malloc(sizeof(DListNode));
    if (newnode == NULL)
    {
        perror("双向链表:创建新节点失败,malloc申请内存失败!");
        return NULL;
    }
    newnode->data = data;
    newnode->prev = NULL;
    newnode->next = NULL;
    return newnode;
}

/**
 * @brief 创建双向链表
 *
 * @param head 传入的头指针
 * @param data 初始数据
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool creat_dlist(DListNode **head, ListData data)
{
    if (*head != NULL)
    {
        fprintf(stderr, "双向链表:创建双向链表失败,链表已经存在!\n");
        return false;
    }
    *head = __creat_newnode(data);
    if (*head == NULL)
    {
        return false;
    }
    return true;
}

/**
 * @brief 头插法插入节点
 *
 * @param head 传入的头指针
 * @param data 要插入的数据
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool head_add_dlist(DListNode **head, ListData data)
{
    DListNode *newnode;

    if (*head == NULL)
    {
        fprintf(stderr, "双向链表:头插法插入节点失败,传入指针为空,已为您自动创建新链表!\n");
        return creat_dlist(head, data);
    }
    newnode = __creat_newnode(data);
    if (newnode == NULL)
    {
        return false;
    }
    newnode->next = *head;
    (*head)->prev = newnode;
    *head = newnode;
    return true;
}

/**
 * @brief 尾插法插入节点
 *
 * @param head 传入的头指针
 * @param data 要插入的数据
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool tail_add_dlist(DListNode **head, ListData data)
{
    DListNode *newnode;
    DListNode *p;

    if (*head == NULL)
    {
        fprintf(stderr, "双向链表:尾插法插入节点失败,传入指针为空,已为您自动创建新链表!\n");
        return creat_dlist(head, data);
    }
    newnode = __creat_newnode(data);
    if (newnode == NULL)
    {
        return false;
    }
    p = *head;
    while (p->next != NULL)
    {
        p = p->next;
    }
    p->next = newnode;
    newnode->prev = p;
    return true;
}

/**
 * @brief 指定位置插入数据
 *
 * @param head 传入的头指针
 * @param pos 指定位置
 * @param data 要插入的数据
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool rand_add_dlist(DListNode **head, ListData pos, ListData data)
{
    DListNode *newnode;
    DListNode *p;

    if (*head == NULL)
    {
        fprintf(stderr, "双向链表:指定位置插入数据失败,传入的指针为空指针,已自动创建新节点!\n");
        return creat_dlist(head, data);
    }
    p = *head;
    while (p != NULL && p->data != pos)
    {
        p = p->next;
    }
    if (p == NULL)
    {
        fprintf(stderr, "双向链表:指定位置插入数据失败,指定数据不存在!\n");
        return false;
    }
    newnode = __creat_newnode(data);
    if (newnode == NULL)
    {
        return false;
    }
    newnode->next = p->next;
    newnode->prev = p;
    if (p->next != NULL)
    {
        p->next->prev = newnode;
    }
    p->next = newnode;
    return true;
}

/**
 * @brief 遍历链表(打印链表)
 *
 * @param head 传入的头指针
 * 特别提醒:如果你要使用自定义数据类型,请修改此函数!
 */
LISTANDTREE_WEAK void printf_dlist(const DListNode *head)
{
    DListNode *p;

    if (head == NULL)
    {
        fprintf(stderr, "双向链表:遍历链表错误,传入指针为空指针!\n");
        return;
    }
    p = (DListNode *)head;
    printf("双向链表遍历:");
    while (p != NULL)
    {
        printf("%d  ", p->data);
        p = p->next;
    }
    printf("\n");
}

/**
 * @brief 查找指定数据的节点
 *
 * @param head 待查询的链表
 * @param data 待查询的数据
 * @return 成功返回节点指针，失败返回NULL
 */
LISTANDTREE_WEAK DListNode *find_node_dlist(const DListNode *head, ListData data)
{
    DListNode *p;

    if (head == NULL)
    {
        fprintf(stderr, "双向链表:查找指定数据的节点失败,传入指针为空!\n");
        return NULL;
    }
    p = (DListNode *)head;
    while (p != NULL)
    {
        if (p->data == data)
        {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

/**
 * @brief 更新节点数据
 *
 * @param head 传入的头指针
 * @param old_data 待操作的节点数据
 * @param new_data 等待更新的目标数据
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool update_node_dlist(DListNode *head, ListData old_data, ListData new_data)
{
    DListNode *p;

    if (head == NULL)
    {
        fprintf(stderr, "双向链表:更新节点数据错误,传入指针为空!\n");
        return false;
    }
    p = head;
    while (p != NULL)
    {
        if (p->data == old_data)
        {
            p->data = new_data;
            return true;
        }
        p = p->next;
    }
    return false;
}

/**
 * @brief 删除指定数据的节点
 *
 * @param head 传入的头指针
 * @param data 要删除的数据
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool del_node_dlist(DListNode **head, ListData data)
{
    DListNode *p;

    if (*head == NULL)
    {
        fprintf(stderr, "双向链表:删除指定数据的节点失败,传入指针为空指针!\n");
        return false;
    }
    p = *head;
    while (p != NULL && p->data != data)
    {
        p = p->next;
    }
    if (p == NULL)
    {
        fprintf(stderr, "双向链表:删除指定数据的节点失败,未找到匹配数据的节点!\n");
        return false;
    }
    if (p->prev == NULL)
    {
        *head = p->next;
        if (*head != NULL)
        {
            (*head)->prev = NULL;
        }
        free(p);
        return true;
    }
    p->prev->next = p->next;
    if (p->next != NULL)
    {
        p->next->prev = p->prev;
    }
    free(p);
    p = NULL;
    return true;
}

/**
 * @brief 销毁链表
 *
 * @param head 传入的头指针
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool del_all_node_dlist(DListNode **head)
{
    DListNode *q = NULL;
    DListNode *p;

    if (*head == NULL)
    {
        fprintf(stderr, "双向链表:销毁链表失败,传入指针为空指针!\n");
        return false;
    }
    p = *head;
    while (p != NULL)
    {
        q = p->next;
        free(p);
        p = q;
    }
    *head = NULL;
    return true;
}
