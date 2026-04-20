#include "tree_bst.h"
#include "listandtree_export.h"

// 创建新节点
static BSTNode *__creat_newnode(ListData data)
{
    BSTNode *newnode = (BSTNode *)malloc(sizeof(BSTNode));
    if (newnode == NULL)
    {
        perror("二叉搜索树:创建新节点失败,malloc申请内存失败!");
        return NULL;
    }
    newnode->data = data;
    newnode->left = NULL;
    newnode->right = NULL;
    return newnode;
}

// 递归销毁二叉搜索树
static void __del_tree(BSTNode *root)
{
    if (root == NULL)
    {
        return;
    }
    __del_tree(root->left);
    __del_tree(root->right);
    free(root);
}

// 中序遍历打印
static void __printf_tree_bst_mid(const BSTNode *root)
{
    if (root == NULL)
    {
        return;
    }
    __printf_tree_bst_mid(root->left);
    printf("%d  ", root->data);
    __printf_tree_bst_mid(root->right);
}

// 前序遍历打印
static void __printf_tree_bst_front(const BSTNode *root)
{
    if (root == NULL)
    {
        return;
    }
    printf("%d  ", root->data);
    __printf_tree_bst_front(root->left);
    __printf_tree_bst_front(root->right);
}

// 后序遍历打印
static void __printf_tree_bst_back(const BSTNode *root)
{
    if (root == NULL)
    {
        return;
    }
    __printf_tree_bst_back(root->left);
    __printf_tree_bst_back(root->right);
    printf("%d  ", root->data);
}

/**
 * @brief 创建二叉搜索树
 *
 * @param root 传入的根指针
 * @param data 初始数据
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool creat_tree_bst(BSTNode **root, ListData data)
{
    if (*root != NULL)
    {
        fprintf(stderr, "二叉搜索树:创建二叉搜索树失败,树已经存在!\n");
        return false;
    }
    *root = __creat_newnode(data);
    if (*root == NULL)
    {
        return false;
    }
    return true;
}

/**
 * @brief 插入节点
 *
 * @param root 传入的根指针
 * @param data 要插入的数据
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool add_node_tree_bst(BSTNode **root, ListData data)
{
    if (*root == NULL)
    {
        fprintf(stderr, "二叉搜索树:插入节点失败,传入指针为空,已为您自动创建新树!\n");
        return creat_tree_bst(root, data);
    }
    if (data < (*root)->data)
    {
        if ((*root)->left == NULL)
        {
            (*root)->left = __creat_newnode(data);
            return (*root)->left != NULL;
        }
        return add_node_tree_bst(&(*root)->left, data);
    }
    if ((*root)->right == NULL)
    {
        (*root)->right = __creat_newnode(data);
        return (*root)->right != NULL;
    }
    return add_node_tree_bst(&(*root)->right, data);
}

/**
 * @brief 查找指定数据的节点
 *
 * @param root 待查询的树
 * @param data 待查询的数据
 * @return 成功返回节点指针，失败返回NULL
 */
LISTANDTREE_WEAK BSTNode *find_node_tree_bst(const BSTNode *root, ListData data)
{
    if (root == NULL)
    {
        return NULL;
    }
    if (root->data == data)
    {
        return (BSTNode *)root;
    }
    if (data < root->data)
    {
        return find_node_tree_bst(root->left, data);
    }
    return find_node_tree_bst(root->right, data);
}

/**
 * @brief 中序遍历二叉搜索树
 *
 * @param root 传入的根指针
 */
LISTANDTREE_WEAK void printf_tree_bst_mid(const BSTNode *root)
{
    if (root == NULL)
    {
        fprintf(stderr, "二叉搜索树:中序遍历错误,传入指针为空指针!\n");
        return;
    }
    printf("二叉搜索树中序遍历:");
    __printf_tree_bst_mid(root);
    printf("\n");
}

/**
 * @brief 前序遍历二叉搜索树
 *
 * @param root 传入的根指针
 */
LISTANDTREE_WEAK void printf_tree_bst_front(const BSTNode *root)
{
    if (root == NULL)
    {
        fprintf(stderr, "二叉搜索树:前序遍历错误,传入指针为空指针!\n");
        return;
    }
    printf("二叉搜索树前序遍历:");
    __printf_tree_bst_front(root);
    printf("\n");
}

/**
 * @brief 后序遍历二叉搜索树
 *
 * @param root 传入的根指针
 */
LISTANDTREE_WEAK void printf_tree_bst_back(const BSTNode *root)
{
    if (root == NULL)
    {
        fprintf(stderr, "二叉搜索树:后序遍历错误,传入指针为空指针!\n");
        return;
    }
    printf("二叉搜索树后序遍历:");
    __printf_tree_bst_back(root);
    printf("\n");
}

/**
 * @brief 删除指定数据的节点
 *
 * @param root 传入的根指针
 * @param data 要删除的数据
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool del_node_tree_bst(BSTNode **root, ListData data)
{
    if (*root == NULL)
    {
        fprintf(stderr, "二叉搜索树:删除指定数据的节点失败,传入指针为空指针或数据不存在!\n");
        return false;
    }
    if (data < (*root)->data)
    {
        return del_node_tree_bst(&(*root)->left, data);
    }
    if (data > (*root)->data)
    {
        return del_node_tree_bst(&(*root)->right, data);
    }
    {
        BSTNode *temp = *root;

        if ((*root)->left == NULL && (*root)->right == NULL)
        {
            *root = NULL;
            free(temp);
            return true;
        }
        if ((*root)->left == NULL)
        {
            *root = (*root)->right;
            free(temp);
            return true;
        }
        if ((*root)->right == NULL)
        {
            *root = (*root)->left;
            free(temp);
            return true;
        }
        else
        {
            BSTNode *parent = *root;
            BSTNode *node = (*root)->right;

            while (node->left != NULL)
            {
                parent = node;
                node = node->left;
            }
            (*root)->data = node->data;
            if (parent == *root)
            {
                parent->right = node->right;
            }
            else
            {
                parent->left = node->right;
            }
            free(node);
        }
    }
    return true;
}

/**
 * @brief 销毁二叉搜索树
 *
 * @param root 传入的根指针
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool del_all_node_tree_bst(BSTNode **root)
{
    if (*root == NULL)
    {
        fprintf(stderr, "二叉搜索树:销毁二叉搜索树失败,传入指针为空指针!\n");
        return false;
    }
    __del_tree(*root);
    *root = NULL;
    return true;
}
