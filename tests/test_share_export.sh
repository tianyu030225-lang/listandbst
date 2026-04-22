#!/usr/bin/env bash
set -euo pipefail

share_app="${1:?usage: test_share_export.sh /abs/path/to/listandtree.out}"
tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT

cp "$share_app" "$tmpdir/listandtree.out"
chmod +x "$tmpdir/listandtree.out"

(
  cd "$tmpdir"
  ./listandtree.out >/tmp/listandtree_share_export.log

  test -d "listandtree"
  test -f "listandtree/liblistandtree.a"
  test -f "listandtree/listandtree.h"
  test -f "README.md"
  test -f "Makefile"

  if find "listandtree" -type f \( -name '*.c' -o -name 'listandtree_export.h' \) | grep -q .; then
    echo "share export unexpectedly contains source files" >&2
    exit 1
  fi

  cat > demo.c <<'EOF'
#include <stdio.h>

#include "listandtree/listandtree.h"

int main(void)
{
    ListNode *head = NULL;
    if (!creat_slist(&head, 11))
    {
        return 1;
    }
    if (!tail_add_slist(&head, 22))
    {
        return 1;
    }
    printf_slist(head);
    del_all_node_slist(&head);
    return 0;
}
EOF

  make SRC=demo.c OUT=demo run >/tmp/listandtree_share_demo.log
  grep -q '11  22' /tmp/listandtree_share_demo.log
)
