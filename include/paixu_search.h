// 排序与查找
#ifndef PAIXU_SEARCH_H
#define PAIXU_SEARCH_H

#include "slist.h"

/**
 * @brief 冒泡排序
 *
 * @param arr 待排序数组
 * @param len 数组长度
 * @return true 成功返回
 * @return false 失败返回
 */
bool bubble_sort(ListData arr[], int len);

/**
 * @brief 快速排序
 *
 * @param arr 待排序数组
 * @param left 左边下标
 * @param right 右边下标
 * @return true 成功返回
 * @return false 失败返回
 */
bool quick_sort(ListData arr[], int left, int right);

/**
 * @brief 二分查找
 *
 * @param arr 有序数组
 * @param len 数组长度
 * @param data 要查找的数据
 * @return 成功返回下标,失败返回-1
 */
int binary_search(ListData arr[], int len, ListData data);

/**
 * @brief 打印数组
 *
 * @param arr 待打印数组
 * @param len 数组长度
 */
void printf_arr(ListData arr[], int len);

#endif
