#!/usr/bin/env python3

from __future__ import annotations

import re
import sys
from pathlib import Path


GROUP_RE = re.compile(r'^API_GROUP\((\w+),\s*"([^"]+)",\s*"([^"]+)"\)$')
ENTRY_RE = re.compile(r'^API_ENTRY\((\w+),\s*"([^"]+)",\s*"([^"]+)"\)$')


def parse_api_catalog(path: Path) -> tuple[list[tuple[str, str, str]], list[tuple[str, str, str]]]:
    groups: list[tuple[str, str, str]] = []
    entries: list[tuple[str, str, str]] = []

    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line:
            continue
        group_match = GROUP_RE.match(line)
        if group_match:
            groups.append(group_match.groups())
            continue
        entry_match = ENTRY_RE.match(line)
        if entry_match:
            entries.append(entry_match.groups())

    return groups, entries


def render_readme(groups: list[tuple[str, str, str]], entries: list[tuple[str, str, str]]) -> str:
    grouped_entries: dict[str, list[tuple[str, str]]] = {group_id: [] for group_id, _, _ in groups}

    for group_id, name, summary in entries:
        grouped_entries[group_id].append((name, summary))

    lines = [
        "# LIST and BST 导出说明",
        "",
        "这是 `listandtree.out` 自动生成的分享版根目录说明文件。",
        "",
        "## 已生成的内容",
        "",
        "- `./listandtree/`: 平铺的头文件、源文件和库用 `Makefile`。",
        "- `./README.md`: 当前导出包的使用方式与 API 总览。",
        "- `./Makefile`: 根目录通用构建入口，用来编译当前目录下的用户 `.c` 文件。",
        "",
        "## 使用方式",
        "",
        "先运行分享版导出器，再在当前目录编写你的用户程序，例如：",
        "",
        "```c",
        '#include "listandtree/listandtree.h"',
        "```",
        "",
        "假设你的文件叫 `test01.c`，则可以直接执行：",
        "",
        "```bash",
        "make run",
        "```",
        "",
        "如果当前目录有多个 `.c` 文件，也可以指定：",
        "",
        "```bash",
        "make SRC=demo.c OUT=demo run",
        "```",
        "",
        "## 同名函数覆盖说明",
        "",
        "- C 不支持函数重载。",
        "- 当前导出包面向 Linux/GCC 或兼容的 Clang ELF 环境，库实现使用 weak symbol 导出。",
        "- 如果你在自己的 `.c` 文件里写了同名函数，链接时会优先使用你的强符号实现。",
        "- 这意味着你既可以直接调用自己的同名函数，也可以在调用库里其他函数时让它们落到你的覆盖实现。",
        "- 如果以后要扩展到非 ELF/非 GCC 平台，再考虑前缀化命名方案。",
        "",
        "## 公开头文件",
        "",
        "- `listandtree.h`",
        "- `slist.h`",
        "- `slist_while.h`",
        "- `dlist.h`",
        "- `dlist_while.h`",
        "- `tree_bst.h`",
        "- `paixu_search.h`",
        "",
        f"## API 总数",
        "",
        f"- 当前导出 API 总数: {len(entries)}",
        "",
    ]

    for group_id, group_name, subtitle in groups:
        lines.extend(
            [
                f"## {group_name}",
                "",
                subtitle,
                "",
                "| 函数 | 作用 |",
                "| --- | --- |",
            ]
        )
        for name, summary in grouped_entries[group_id]:
            lines.append(f"| `{name}` | {summary} |")
        lines.append("")

    return "\n".join(lines)


def to_c_array(data: bytes) -> str:
    parts = []
    for index, byte in enumerate(data):
        if index % 12 == 0:
            parts.append("\n    ")
        parts.append(f"0x{byte:02x}, ")
    if not parts:
        return "\n    "
    return "".join(parts)


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: build_share_assets.py <output.c> <output.h>", file=sys.stderr)
        return 1

    root = Path(__file__).resolve().parents[1]
    output_c = Path(sys.argv[1])
    output_h = Path(sys.argv[2])
    catalog_path = root / "src" / "api_catalog.inc"
    groups, entries = parse_api_catalog(catalog_path)
    readme_content = render_readme(groups, entries).encode("utf-8")
    root_makefile_content = (root / "share" / "user_makefile").read_bytes()

    manifest = [
        ("Makefile", root / "share" / "export_makefile"),
        ("listandtree_export.h", root / "src" / "listandtree_export.h"),
        ("slist.h", root / "include" / "slist.h"),
        ("slist_while.h", root / "include" / "slist_while.h"),
        ("dlist.h", root / "include" / "dlist.h"),
        ("dlist_while.h", root / "include" / "dlist_while.h"),
        ("tree_bst.h", root / "include" / "tree_bst.h"),
        ("paixu_search.h", root / "include" / "paixu_search.h"),
        ("listandtree.h", root / "include" / "listandtree.h"),
        ("slist.c", root / "src" / "slist.c"),
        ("slist_while.c", root / "src" / "slist_while.c"),
        ("dlist.c", root / "src" / "dlist.c"),
        ("dlist_while.c", root / "src" / "dlist_while.c"),
        ("tree_bst.c", root / "src" / "tree_bst.c"),
        ("paixu_search.c", root / "src" / "paixu_search.c"),
    ]

    header_lines = [
        "#ifndef SHARE_ASSETS_H",
        "#define SHARE_ASSETS_H",
        "",
        "#include <stddef.h>",
        "",
        "typedef struct",
        "{",
        "    const char *relative_path;",
        "    const unsigned char *data;",
        "    size_t size;",
        "} ShareAsset;",
        "",
        "extern const ShareAsset SHARE_ASSETS[];",
        "extern const size_t SHARE_ASSET_COUNT;",
        "extern const unsigned char SHARE_README_DATA[];",
        "extern const size_t SHARE_README_SIZE;",
        "extern const unsigned char SHARE_ROOT_MAKEFILE_DATA[];",
        "extern const size_t SHARE_ROOT_MAKEFILE_SIZE;",
        "",
        "#endif",
        "",
    ]

    c_lines = [
        '#include "share_assets.h"',
        "",
    ]

    asset_names: list[str] = []
    for index, (relative_path, source_path) in enumerate(manifest):
        array_name = f"share_asset_{index}"
        asset_names.append(array_name)
        data = source_path.read_bytes()
        c_lines.extend(
            [
                f"static const unsigned char {array_name}[] = {{{to_c_array(data)}\n}};",
                "",
            ]
        )

    c_lines.extend(
        [
            f"const unsigned char SHARE_README_DATA[] = {{{to_c_array(readme_content)}\n}};",
            f"const size_t SHARE_README_SIZE = sizeof(SHARE_README_DATA);",
            "",
            f"const unsigned char SHARE_ROOT_MAKEFILE_DATA[] = {{{to_c_array(root_makefile_content)}\n}};",
            f"const size_t SHARE_ROOT_MAKEFILE_SIZE = sizeof(SHARE_ROOT_MAKEFILE_DATA);",
            "",
            "const ShareAsset SHARE_ASSETS[] = {",
        ]
    )

    for index, (relative_path, _) in enumerate(manifest):
        c_lines.append(
            f'    {{"{relative_path}", {asset_names[index]}, sizeof({asset_names[index]})}},'
        )

    c_lines.extend(
        [
            "};",
            f"const size_t SHARE_ASSET_COUNT = sizeof(SHARE_ASSETS) / sizeof(SHARE_ASSETS[0]);",
            "",
        ]
    )

    output_h.write_text("\n".join(header_lines), encoding="utf-8")
    output_c.write_text("\n".join(c_lines), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
