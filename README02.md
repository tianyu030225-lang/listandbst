# README02：当前版本实现详解

这份文档说明 `myproject01_4.0` 当前版本的真实定位、模块关系和导出方式。

## 1. 当前定位

当前工程保留两条产物线：

- 开发版实验室 `bin/listandtree_lab`
- 分享版导出器 `dist/listandtree.out`

开发版额外会维护本地运行态目录：

- `runtime_data/`

## 2. 当前公开接口

公开头文件已经切换为：

- `include/slist.h`
- `include/slist_while.h`
- `include/dlist.h`
- `include/dlist_while.h`
- `include/tree_bst.h`
- `include/paixu_search.h`
- `include/listandtree.h`

其中：

- `listandtree.h` 只负责聚合包含这 6 组头文件
- 仓库根目录额外提供一个转发头 `listandtree.h`，便于外部项目直接写 `#include "listandbst/listandtree.h"`
- 所有接口都直接暴露节点指针或整型数组
- 不再保留之前 4.0 那套容器对象式 API

## 3. API 清单与总数

统一 API 清单仍由：

- `src/api_catalog.inc`
- `src/api_catalog.h`
- `src/api_catalog.c`

负责。

它现在同时服务于：

- 开发版 API 总览页
- 分享版自动生成的 `README.md`
- 分享版完成页的 API 数量提示

当前总数为：

- `48` 个函数

按模块拆分为：

- `SList`：9 个
- `SListWhile`：9 个
- `DList`：9 个
- `DListWhile`：9 个
- `TreeBST`：8 个
- `PaiXuSearch`：4 个

## 4. 显示层如何工作

显示层仍由：

- `include/show.h`
- `src/show.c`

负责，品牌仍然固定为：

- `LIST and BST`

本次只调整了启动和导出流程，不改菜单壳子：

- 启动页恢复为 3 条逐条加载动画
- 导出页恢复为 3 条逐条加载动画
- 开发版动画完成后会真实等待 `按回车进入主菜单...`
- 非 TTY 或 `LISTANDTREE_FAST_UI=1` 时直接走快速模式

保留内容包括：

- 顶部会话壳子
- 主菜单
- 页面标题
- 消息提示
- 退出页
- 作者信息展示

## 5. 开发版主程序如何组织

开发版入口：

- `src/main.c`

默认构建目标：

- `bin/listandtree_lab`

主程序现在负责：

- 显示启动欢迎页
- 执行 3 步启动检查
- 启动时恢复 `runtime_data/` 下的上次链表/BST 数据
- 进入菜单循环
- 调用你原始 API 的真实交互
- 正常退出前把当前链表/BST 数据随机打乱后再持久化写回本地
- 展示统一 API 清单
- 展示构建、测试和项目结构说明

菜单页已经全部改回原始函数调用方式，例如：

- 四种链表页都改成用户手动操作台，不再预先写死样本数据
- 链表页使用 `creat_* / head_add_* / tail_add_* / rand_add_* / find_* / update_* / del_* / printf_*`
- BST 页改成用户手动操作台，使用 `creat_tree_bst / add_node_tree_bst / find_node_tree_bst / printf_tree_bst_* / del_node_tree_bst`
- 排序页继续使用 `bubble_sort / quick_sort / binary_search / printf_arr`

当前交互方式是：

- 进入四种链表页面后，由用户自己决定何时创建首节点、头插、尾插、在指定数据后插入、打印、查找、更新、删除、销毁
- 进入 BST 页面后，由用户自己决定何时创建根节点、插入、查找、输出三种遍历、删除、销毁
- 每次操作完成后停在 `按回车继续操作...`，返回主菜单时不再额外重复停顿

## 6. 分享版导出器如何工作

分享版入口：

- `src/share_main.c`

目标产物：

- `dist/listandtree.out`

只有执行：

```bash
make share
```

时才会生成分享版。

运行分享版后，会在当前目录尝试写出：

- `./listandtree/`
- `./README.md`
- `./Makefile`

当前策略是：

- `listandtree/` 不存在时才新建并写入公开头文件与 `liblistandtree.a`
- `README.md` 不存在时才新建
- 根目录 `Makefile` 不存在时才新建

导出页会严格按 4 步执行并回报结果：

1. 检查或创建导出目录
2. 写出分享库资源
3. 写出 `README.md`
4. 写出根目录 `Makefile`

## 7. 分享资源打包方式

构建分享版时，`tools/build_share_assets.py` 会：

1. 先把 6 个实现文件编译并归档为 `liblistandtree.a`
2. 读取 6 组公开头文件与 `share/user_makefile`
3. 根据 `api_catalog.inc` 自动生成根目录 `README.md`
4. 生成 `build/share_assets.c` / `build/share_assets.h`
5. 编译进 `dist/listandtree.out`

因此 `listandtree.out` 生成后就是一个固定的二进制分享快照，不包含源码。

## 8. 同名函数覆盖方案

当前版本没有改公开头文件里的函数名，也没有在公开头文件里引入 `weak` 宏。

实际做法是：

- 新增 `src/listandtree_export.h` 作为库实现私有头
- 只在 6 个库实现文件内部把 48 个公开 API 定义标成 `weak`
- 用户源文件里自己写的同名函数保持强符号
- 链接时优先采用用户实现，库内其他函数对这些符号的调用也会落到用户实现

这里要明确：

- C 不支持函数重载
- 当前 Linux/GCC 或兼容 Clang 的 ELF 环境采用的是 weak symbol 覆盖
- 如果未来要支持非 ELF/非 GCC 平台，再考虑前缀化命名方案

## 9. 测试与回归

测试入口保持为：

- `tests/test_lists.c`
- `tests/test_bst.c`
- `tests/test_algorithms.c`
- `tests/test_runtime_store.c`
- `tests/test_share_export.sh`

并新增了两个约束：

- `make valgrind` 用于统一跑泄漏检查
- 循环链表和双向循环链表都要覆盖单节点/多节点回归路径

## 10. 当前目录结构

- `include/`: 原始 API 头文件、`listandtree.h`、`show.h`
- `src/`: 开发版入口、分享版入口、显示层、六组实现、API 清单、私有导出宏头
- `tests/`: 单元测试、持久化测试、分享导出验证
- `share/`: 用户根目录 Makefile 模板
- `tools/`: 分享资源打包脚本
- `dist/`: 分享版输出目录

当前工程重点已经明确为：

- 原始 API 版本的数据结构实验室
- 分阶段快启的交互壳子
- 可重复导出的预编译库分享版
