#include "slist_while.h"
#include "listandtree_export.h"

// 创建新节点
static SListWhileNode *__creat_newnode(ListData data)
{
    SListWhileNode *newnode = (SListWhileNode *)malloc(sizeof(SListWhileNode));
    if (newnode == NULL)
    {
        perror("单向循环链表:创建新节点失败,malloc申请内存失败!");
        return NULL;
    }
    newnode->data = data;
    newnode->next = newnode;
    return newnode;
}

/**
 * @brief 创建单向循环链表
 *
 * @param head 传入的头指针
 * @param data 初始数据
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool creat_slist_while(SListWhileNode **head, ListData data)
{
    if (*head != NULL)
    {
        fprintf(stderr, "单向循环链表:创建单向循环链表失败,链表已经存在!\n");
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
LISTANDTREE_WEAK bool head_add_slist_while(SListWhileNode **head, ListData data)
{
    SListWhileNode *newnode;
    SListWhileNode *tail;

    if (*head == NULL)
    {
        fprintf(stderr, "单向循环链表:头插法插入节点失败,传入指针为空,已为您自动创建新链表!\n");
        return creat_slist_while(head, data);
    }
    newnode = __creat_newnode(data);
    if (newnode == NULL)
    {
        return false;
    }
    tail = *head;
    while (tail->next != *head)
    {
        tail = tail->next;
    }
    newnode->next = *head;
    tail->next = newnode;
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
LISTANDTREE_WEAK bool tail_add_slist_while(SListWhileNode **head, ListData data)
{
    SListWhileNode *newnode;
    SListWhileNode *tail;

    if (*head == NULL)
    {
        fprintf(stderr, "单向循环链表:尾插法插入节点失败,传入指针为空,已为您自动创建新链表!\n");
        return creat_slist_while(head, data);
    }
    newnode = __creat_newnode(data);
    if (newnode == NULL)
    {
        return false;
    }
    tail = *head;
    while (tail->next != *head)
    {
        tail = tail->next;
    }
    newnode->next = *head;
    tail->next = newnode;
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
LISTANDTREE_WEAK bool rand_add_slist_while(SListWhileNode **head, ListData pos, ListData data)
{
    SListWhileNode *newnode;
    SListWhileNode *p;

    if (*head == NULL)
    {
        fprintf(stderr, "单向循环链表:指定位置插入数据失败,传入的指针为空指针,已自动创建新节点!\n");
        return creat_slist_while(head, data);
    }
    p = *head;
    do
    {
        if (p->data == pos)
        {
            break;
        }
        p = p->next;
    } while (p != *head);
    if (p->data != pos)
    {
        fprintf(stderr, "单向循环链表:指定位置插入数据失败,指定数据不存在!\n");
        return false;
    }
    newnode = __creat_newnode(data);
    if (newnode == NULL)
    {
        return false;
    }
    newnode->next = p->next;
    p->next = newnode;
    return true;
}

/**
 * @brief 遍历链表(打印链表)
 *
 * @param head 传入的头指针
 * 特别提醒:如果你要使用自定义数据类型,请修改此函数!
 */
LISTANDTREE_WEAK void printf_slist_while(const SListWhileNode *head)
{
    SListWhileNode *p;

    if (head == NULL)
    {
        fprintf(stderr, "单向循环链表:遍历链表错误,传入指针为空指针!\n");
        return;
    }
    p = (SListWhileNode *)head;
    printf("单向循环链表遍历:");
    do
    {
        printf("%d  ", p->data);
        p = p->next;
    } while (p != head);
    printf("\n");
}

/**
 * @brief 查找指定数据的节点
 *
 * @param head 待查询的链表
 * @param data 待查询的数据
 * @return 成功返回节点指针，失败返回NULL
 */
LISTANDTREE_WEAK SListWhileNode *find_node_slist_while(const SListWhileNode *head, ListData data)
{
    SListWhileNode *p;

    if (head == NULL)
    {
        fprintf(stderr, "单向循环链表:查找指定数据的节点失败,传入指针为空!\n");
        return NULL;
    }
    p = (SListWhileNode *)head;
    do
    {
        if (p->data == data)
        {
            return p;
        }
        p = p->next;
    } while (p != head);
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
LISTANDTREE_WEAK bool update_node_slist_while(SListWhileNode *head, ListData old_data, ListData new_data)
{
    SListWhileNode *p;

    if (head == NULL)
    {
        fprintf(stderr, "单向循环链表:更新节点数据错误,传入指针为空!\n");
        return false;
    }
    p = head;
    do
    {
        if (p->data == old_data)
        {
            p->data = new_data;
            return true;
        }
        p = p->next;
    } while (p != head);
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
LISTANDTREE_WEAK bool del_node_slist_while(SListWhileNode **head, ListData data)
{
    SListWhileNode *q = NULL;
    SListWhileNode *p;

    if (*head == NULL)
    {
        fprintf(stderr, "单向循环链表:删除指定数据的节点失败,传入指针为空指针!\n");
        return false;
    }
    p = *head;
    do
    {
        if (p->data == data)
        {
            break;
        }
        q = p;
        p = p->next;
    } while (p != *head);
    if (p->data != data)
    {
        fprintf(stderr, "单向循环链表:删除指定数据的节点失败,未找到匹配数据的节点!\n");
        return false;
    }
    if (p == *head && p->next == *head)
    {
        free(p);
        *head = NULL;
        return true;
    }
    if (q == NULL)
    {
        SListWhileNode *tail = *head;
        while (tail->next != *head)
        {
            tail = tail->next;
        }
        *head = p->next;
        tail->next = *head;
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
LISTANDTREE_WEAK bool del_all_node_slist_while(SListWhileNode **head)
{
    SListWhileNode *p;

    if (*head == NULL)
    {
        fprintf(stderr, "单向循环链表:销毁链表失败,传入指针为空指针!\n");
        return false;
    }
    p = (*head)->next;
    while (p != *head)
    {
        SListWhileNode *q = p->next;
        free(p);
        p = q;
    }
    free(*head);
    *head = NULL;
    return true;
}
