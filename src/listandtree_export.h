#ifndef LISTANDTREE_EXPORT_H
#define LISTANDTREE_EXPORT_H

#if defined(__GNUC__) || defined(__clang__)
#define LISTANDTREE_WEAK __attribute__((weak))
#else
#define LISTANDTREE_WEAK
#endif

#endif
