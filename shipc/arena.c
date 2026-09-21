#include <stdio.h>
#include "arena.h"

 void arena_init(Arena* a, size_t cap) {
    a->base = (char*) malloc(cap);
    a->used = 0;
    a->cap = cap;
}

void* arena_alloc(Arena* a, size_t size) {
    size = (size + 7) & ~(size_t)7;
    if (a->used + size > a->cap) {
        fprintf(stderr, "AST arena exhausted\n");
        exit(1);
    }

    void* p = a->base + a->used;
    a->used += size;
    return p;
}

void arena_free(Arena* a) {
    free(a->base);
}
