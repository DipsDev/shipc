#include <string.h>
#include "node.h"



Node* node_new (Arena* arena, NodeType type, Token tkn) {
    Node* n = (Node*) arena_alloc(arena, sizeof(Node));
    memset(n, 0, sizeof(Node));
    n->type = type;
    n->token = tkn;
    return n;
}

void node_list_push(Arena* arena, NodeList* list, Node* item) {
    if (list->count == list->cap) {
        int new_cap = list->cap ? list->cap * 2 : 4;
        Node** grown = (Node**) arena_alloc(arena, sizeof(Node*) * new_cap);
        if (list->count) memcpy(grown, list->items, sizeof(Node*) * list->count);
        list->items = grown;
        list->cap = new_cap;
    }
    list->items[list->count++] = item;
}

