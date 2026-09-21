#ifndef SHIPC_ARENA_H
#define SHIPC_ARENA_H

#include "stdlib.h"

typedef struct Arena {
    char* base;
    size_t used;
    size_t cap;
} Arena;


void arena_init(Arena* a, size_t cap);
void* arena_alloc(Arena* a, size_t size);
void arena_free(Arena* a);

#endif //SHIPC_ARENA_H
