# LIST and BST

这是一个运行在 Linux 下的 C 语言双产物项目：

- 开发版：带欢迎页、菜单和演示页面的 `LIST and BST` 实验室
- 分享版：单文件导出器 `listandtree.out`

当前版本已经回切为你在 `myproject01` 里写的原始 API 风格，对外公开 `48` 个函数。

## 公开模块

- `SList`
- `SListWhile`
- `DList`
- `DListWhile`
- `TreeBST`
- `PaiXuSearch`

统一总头文件：

```c
#include "listandtree.h"
```

如果你是把整个仓库直接 `git clone` 到外部工程旁边使用，也可以在外部代码里写：

```c
#include "listandbst/listandtree.h"
```

它会一次性包含：

- `slist.h`
- `slist_while.h`
- `dlist.h`
- `dlist_while.h`
- `tree_bst.h`
- `paixu_search.h`

## 构建

开发版：

```bash
make
make run
```

生成：

```bash
./bin/listandtree_lab
```

测试：

```bash
make test
make valgrind
```

如果系统里没有安装 `valgrind`，`make valgrind` 会明确提示跳过。

分享版：

```bash
make share
```

生成：

```bash
./dist/listandtree.out
```

## 开发版菜单

- `[1]` 单向链表操作 `SList`
- `[2]` 单向循环操作 `SListWhile`
- `[3]` 双向链表操作 `DList`
- `[4]` 双向循环操作 `DListWhile`
- `[5]` BST 操作台 `TreeBST`
- `[6]` 排序与查找 `PaiXuSearch`
- `[7]` API 总览
- `[8]` 结构对比
- `[9]` 构建测试
- `[10]` 项目结构

其中 `[1]` 到 `[5]` 已经改成用户手动操作模式：

- 四种链表页面不再提前写死样本数据，而是由用户自己选择创建、头插、尾插、指定数据后插入、打印、查找、更新、删除、销毁
- BST 页面不再提前构造固定树，而是由用户自己选择创建根节点、插入、查找、三种遍历、删除、销毁
- 开发版启动时会自动读取 `runtime_data/` 下的上次数据；正常退出时会把 4 种链表和 BST 的数据随机打乱后再持久化回本地
- 排序与查找页继续保留数组 API 演示，不受这次改动影响
- `[0]` 退出

欢迎页和分享页默认都会恢复逐条加载动画：

- 开发版欢迎页依次显示：
  `正在装载链表实验接口`、
  `正在装载树与排序样本`、
  `正在挂载终端交互外壳`
- 开发版动画完成后会停在 `按回车进入主菜单...`
- 分享版欢迎页依次显示：
  `正在校验分享版导出器`、
  `正在装载分享库资源`、
  `正在整理 README 与构建脚本`
- 分享版动画结束后直接进入导出流程，不额外等待回车
- 非 TTY 或 `LISTANDTREE_FAST_UI=1` 时直接走快速模式

## 分享版行为

`dist/listandtree.out` 是一个自包含的 Linux 可执行文件。

别人把它放到任意文件夹运行后，会在当前目录生成三个东西：

- `./listandtree/`
- `./README.md`
- `./Makefile`

其中：

- `listandtree/` 是平铺的公开头文件目录，并包含预编译静态库 `liblistandtree.a`
- `README.md` 是自动生成的接口与使用说明
- 根目录 `Makefile` 用于让用户写完自己的 `.c` 文件后直接 `make run`
- 分享包不再释放任何 `.c` 源码；如需源码，请自行通过 Git 获取仓库副本

如果当前目录里已经存在 `listandtree/`、`README.md` 或 `Makefile`，默认不会覆盖。

## 分享包使用方式

运行分享版：

```bash
chmod +x listandtree.out
./listandtree.out
```

然后在当前目录写一个用户程序，例如：

```c
#include "listandtree/listandtree.h"
```

假设文件名叫 `test01.c`，则可以直接：

```bash
make run
```

如果当前目录有多个 `.c` 文件，也可以指定：

```bash
make SRC=demo.c OUT=demo run
```

## 同名函数覆盖说明

- C 不支持函数重载。
- 当前 Linux/GCC 版本里，这 48 个库 API 在库实现侧以 `weak symbol` 导出。
- 如果你在自己的源文件里写了同名函数，链接时会优先使用你的强符号实现。
- 这意味着你不仅可以直接调用自己的同名函数，也可以在调用库里的其他函数时让它们落到你的实现上。
- 如果以后要做跨平台支持，再考虑统一前缀化命名方案。

## 测试

当前测试包括：

- `tests/test_lists.c`
- `tests/test_bst.c`
- `tests/test_algorithms.c`
- `tests/test_runtime_store.c`
- `tests/test_share_export.sh`

覆盖内容包括：

- 四种链表的创建、头尾插入、指定位置插入、查找、更新、删除、销毁
- 循环链表和双向循环链表的单节点/多节点回归
- BST 的创建、插入、查找、删除、销毁
- 冒泡排序、快速排序、二分查找和打印接口

## 项目结构

- `include/`: 六组公开头文件、`listandtree.h`、`show.h`
- 仓库根目录 `listandtree.h`: 供外部项目通过 `#include "listandbst/listandtree.h"` 直接引用
- `src/`: 开发版入口、分享版入口、显示层、六组实现、统一 API 清单
- `tests/`: 链表、BST、排序与查找、持久化与分享导出验证
- `share/user_makefile`: 分享版导出的根目录通用 Makefile 模板
- `tools/build_share_assets.py`: 把公开头文件、预编译静态库、根 Makefile 和 README 打进分享版二进制

## 说明

- 默认目标平台是 Linux
- `make` / `make run` 只操作开发版
- `make share` 才会生成分享用的 `listandtree.out`
- `make share-check` 会校验分享版导出目录不含源码且可正常链接 API
- `README02.md` 记录当前版本的实现说明
