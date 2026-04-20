// 二叉搜索树
#ifndef TREE_BST_H
#define TREE_BST_H

#include "slist.h"

// 二叉搜索树结构体
typedef struct bstnode
{
    ListData data;         // 数据域
    struct bstnode *left;  // 左子树指针
    struct bstnode *right; // 右子树指针
} BSTNode;

/**
 * @brief 创建二叉搜索树
 *
 * @param root 传入的根指针
 * @param data 初始数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool creat_tree_bst(BSTNode **root, ListData data);

/**
 * @brief 插入节点
 *
 * @param root 传入的根指针
 * @param data 要插入的数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool add_node_tree_bst(BSTNode **root, ListData data);

/**
 * @brief 查找指定数据的节点
 *
 * @param root 待查询的树
 * @param data 待查询的数据
 * @return 成功返回节点指针，失败返回NULL
 */
BSTNode *find_node_tree_bst(const BSTNode *root, ListData data);

/**
 * @brief 中序遍历二叉搜索树
 *
 * @param root 传入的根指针
 */
void printf_tree_bst_mid(const BSTNode *root);

/**
 * @brief 前序遍历二叉搜索树
 *
 * @param root 传入的根指针
 */
void printf_tree_bst_front(const BSTNode *root);

/**
 * @brief 后序遍历二叉搜索树
 *
 * @param root 传入的根指针
 */
void printf_tree_bst_back(const BSTNode *root);

/**
 * @brief 删除指定数据的节点
 *
 * @param root 传入的根指针
 * @param data 要删除的数据
 * @return true 成功返回
 * @return false 失败返回
 */
bool del_node_tree_bst(BSTNode **root, ListData data);

/**
 * @brief 销毁二叉搜索树
 *
 * @param root 传入的根指针
 * @return true 成功返回
 * @return false 失败返回
 */
bool del_all_node_tree_bst(BSTNode **root);

#endif
