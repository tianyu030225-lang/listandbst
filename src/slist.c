#include "slist.h"
#include "listandtree_export.h"

// 创建新节点
static ListNode *__creat_newnode(ListData data)
{
    ListNode *newnode = (ListNode *)malloc(sizeof(ListNode));
    if (newnode == NULL)
    {
        perror("单向链表:创建新节点失败,malloc申请内存失败!");
        return NULL;
    }
    newnode->data = data;
    newnode->next = NULL;
    return newnode;
}

/**
 * @brief 创建单向链表
 *
 * @param head 传入的头指针
 * @param data 初始数据
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool creat_slist(ListNode **head, ListData data)
{
    if (*head != NULL)
    {
        fprintf(stderr, "单向链表:创建单向链表失败,链表已经存在!\n");
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
LISTANDTREE_WEAK bool head_add_slist(ListNode **head, ListData data)
{
    ListNode *new_node;

    if (*head == NULL)
    {
        fprintf(stderr, "单向链表:头插法插入节点失败,传入指针为空,已为您自动创建新链表!\n");
        return creat_slist(head, data);
    }
    new_node = __creat_newnode(data);
    if (new_node == NULL)
    {
        return false;
    }
    new_node->next = *head;
    *head = new_node;
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
LISTANDTREE_WEAK bool tail_add_slist(ListNode **head, ListData data)
{
    ListNode *new_node;
    ListNode *p;

    if (*head == NULL)
    {
        fprintf(stderr, "单向链表:尾插法插入节点失败,传入指针为空,已为您自动创建新链表!\n");
        return creat_slist(head, data);
    }
    new_node = __creat_newnode(data);
    if (new_node == NULL)
    {
        return false;
    }
    p = *head;
    while (p->next != NULL)
    {
        p = p->next;
    }
    p->next = new_node;
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
LISTANDTREE_WEAK bool rand_add_slist(ListNode **head, ListData pos, ListData data)
{
    ListNode *new_node;
    ListNode *p;

    if (*head == NULL)
    {
        fprintf(stderr, "单向链表:指定位置插入数据失败,传入的指针为空指针,已自动创建新节点!\n");
        return creat_slist(head, data);
    }
    p = *head;
    while (p != NULL && p->data != pos)
    {
        p = p->next;
    }
    if (p == NULL)
    {
        fprintf(stderr, "单向链表:指定位置插入数据失败,指定数据不存在!\n");
        return false;
    }
    new_node = __creat_newnode(data);
    if (new_node == NULL)
    {
        return false;
    }
    new_node->next = p->next;
    p->next = new_node;
    return true;
}

/**
 * @brief 遍历链表(打印链表)
 *
 * @param head 传入的头指针
 * 特别提醒:如果你要使用自定义数据类型,请修改此函数!
 */
LISTANDTREE_WEAK void printf_slist(const ListNode *head)
{
    const ListNode *p;

    if (head == NULL)
    {
        fprintf(stderr, "单向链表:遍历链表错误,传入指针为空指针!\n");
        return;
    }
    p = head;
    printf("单向链表遍历:");
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
LISTANDTREE_WEAK ListNode *find_node_slist(const ListNode *head, ListData data)
{
    const ListNode *p;

    if (head == NULL)
    {
        fprintf(stderr, "单向链表:查找指定数据的节点失败,传入指针为空!\n");
        return NULL;
    }
    p = head;
    while (p != NULL)
    {
        if (p->data == data)
        {
            return (ListNode *)p;
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
LISTANDTREE_WEAK bool update_node_slist(ListNode *head, ListData old_data, ListData new_data)
{
    ListNode *p;

    if (head == NULL)
    {
        fprintf(stderr, "单向链表:更新节点数据错误,传入指针为空!\n");
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
LISTANDTREE_WEAK bool del_node_slist(ListNode **head, ListData data)
{
    ListNode *q = NULL;
    ListNode *p;

    if (*head == NULL)
    {
        fprintf(stderr, "单向链表:删除指定数据的节点失败,传入指针为空指针!\n");
        return false;
    }
    p = *head;
    while (p != NULL && p->data != data)
    {
        q = p;
        p = p->next;
    }
    if (p == NULL)
    {
        fprintf(stderr, "单向链表:删除指定数据的节点失败,未找到匹配数据的节点!\n");
        return false;
    }
    if (q == NULL)
    {
        *head = p->next;
        free(p);
        return true;
    }
    q->next = p->next;
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
LISTANDTREE_WEAK bool del_all_node_slist(ListNode **head)
{
    ListNode *q = NULL;
    ListNode *p;

    if (*head == NULL)
    {
        fprintf(stderr, "单向链表:销毁链表失败,传入指针为空指针!\n");
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
