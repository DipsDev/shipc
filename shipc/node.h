#ifndef SHIPC_NODE_H
#define SHIPC_NODE_H

#include <stdbool.h>
#include "token.h"
#include "arena.h"

typedef enum {
    NODE_NUMBER, NODE_STRING, NODE_LITERAL, NODE_VARIABLE, NODE_ASSIGN,
    NODE_UNARY, NODE_BINARY, NODE_CALL, NODE_ATTR, NODE_ARRAY, NODE_RANGE, NODE_PRINT,
    // statements
    NODE_EXPR_STMT, NODE_VAR, NODE_GLOBAL, NODE_RETURN,
    NODE_IF, NODE_WHILE, NODE_FOR, NODE_FUNC,

} NodeType;

typedef struct Node Node;

typedef struct {
    Node** items;
    int count;
    int cap;
} NodeList;

struct Node {
    NodeType type;
    Token token;   // the identifying token: operator, keyword, literal, or the NAME for
    // variable / assign / attr / var / glob / for / func nodes.
    // Used for line info and for compile-pass error messages.
    union {
        double number;                                  // NODE_NUMBER
        Node*  child;                                   // UNARY, PRINT, ASSIGN, VAR, GLOBAL,
        // ATTR (the object), EXPR_STMT,
        // RETURN (NULL for a bare `return;`)
        NodeList items;                                 // ARRAY
        struct { Node* left; Node* right; } binary;     // BINARY, RANGE
        struct { Node* callee; NodeList args; } call;   // CALL
        struct {
            Node* cond;
            NodeList then_body;
            NodeList else_body;
            bool hasElse;
        } if_stmt;                                      // IF
        struct { Node* cond; NodeList body; } while_stmt;         // WHILE
        struct { Node* iterable; NodeList body; } for_stmt;       // FOR (loop variable is token)
        struct { Token* params; int paramCount; NodeList body; bool is_anon; } func;   // FUNC (name is token)
    } as;
};


Node* node_new (Arena* arena, NodeType type, Token tkn);
void node_list_push(Arena* arena, NodeList* list, Node* item);

#endif //SHIPC_NODE_H
