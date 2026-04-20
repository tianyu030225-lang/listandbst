#include "paixu_search.h"
#include "listandtree_export.h"

// 交换两个数据
static void __swap_data(ListData *a, ListData *b)
{
    ListData temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * @brief 冒泡排序
 *
 * @param arr 待排序数组
 * @param len 数组长度
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool bubble_sort(ListData arr[], int len)
{
    int i;

    if (arr == NULL)
    {
        fprintf(stderr, "排序查找:冒泡排序失败,传入数组为空指针!\n");
        return false;
    }
    if (len <= 1)
    {
        return true;
    }
    for (i = 0; i < len - 1; i++)
    {
        bool flag = false;
        int j;

        for (j = 0; j < len - 1 - i; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                __swap_data(&arr[j], &arr[j + 1]);
                flag = true;
            }
        }
        if (flag == false)
        {
            break;
        }
    }
    return true;
}

/**
 * @brief 快速排序
 *
 * @param arr 待排序数组
 * @param left 左边下标
 * @param right 右边下标
 * @return true 成功返回
 * @return false 失败返回
 */
LISTANDTREE_WEAK bool quick_sort(ListData arr[], int left, int right)
{
    int i;
    int j;
    ListData mid;

    if (arr == NULL)
    {
        fprintf(stderr, "排序查找:快速排序失败,传入数组为空指针!\n");
        return false;
    }
    if (left >= right)
    {
        return true;
    }
    i = left;
    j = right;
    mid = arr[(left + right) / 2];
    while (i <= j)
    {
        while (arr[i] < mid)
        {
            i++;
        }
        while (arr[j] > mid)
        {
            j--;
        }
        if (i <= j)
        {
            __swap_data(&arr[i], &arr[j]);
            i++;
            j--;
        }
    }
    if (left < j)
    {
        quick_sort(arr, left, j);
    }
    if (i < right)
    {
        quick_sort(arr, i, right);
    }
    return true;
}

/**
 * @brief 二分查找
 *
 * @param arr 有序数组
 * @param len 数组长度
 * @param data 要查找的数据
 * @return 成功返回下标,失败返回-1
 */
LISTANDTREE_WEAK int binary_search(ListData arr[], int len, ListData data)
{
    int left = 0;
    int right = len - 1;

    if (arr == NULL)
    {
        fprintf(stderr, "排序查找:二分查找失败,传入数组为空指针!\n");
        return -1;
    }
    while (left <= right)
    {
        int mid = (left + right) / 2;

        if (arr[mid] == data)
        {
            return mid;
        }
        if (arr[mid] > data)
        {
            right = mid - 1;
        }
        else
        {
            left = mid + 1;
        }
    }
    return -1;
}

/**
 * @brief 打印数组
 *
 * @param arr 待打印数组
 * @param len 数组长度
 */
LISTANDTREE_WEAK void printf_arr(ListData arr[], int len)
{
    int i;

    if (arr == NULL)
    {
        fprintf(stderr, "排序查找:打印数组失败,传入数组为空指针!\n");
        return;
    }
    printf("数组遍历:");
    for (i = 0; i < len; i++)
    {
        printf("%d  ", arr[i]);
    }
    printf("\n");
}
