#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "compiler.h"
#include "token.h"
#include "objects.h"
#include "arena.h"
#include "node.h"


typedef struct {
    Token current;
    Token previous;

    Arena arena;

    bool hadError;
    bool panicMode;
} Parser;


// Precedence utils
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;

// A prefix rule runs after its token was consumed. An infix rule additionally
// receives the already-parsed left operand.
typedef Node* (*PrefixFn)(Parser* parser, Scanner* scanner);
typedef Node* (*InfixFn)(Parser* parser, Scanner* scanner, Node* left);

typedef struct {
    PrefixFn prefix;   // before identifier. ex: -4
    InfixFn infix;     // after identifier ex: 4 +
    Precedence precedence;
} ParseRule;

static ParseRule* get_rule(uint8_t token);
static Node* parse_statement(Parser* parser, Scanner* scanner);
static Node* parse_expression(Parser* parser, Scanner* scanner);
static void advance(Scanner* scanner, Parser* parser);
static void synchronize(Parser* parser, Scanner* scanner);


// ---- errors -----------------------------------------------------------

// Prints an error pointing at `tok`. Shared by both passes.
static void print_error(Token tok, const char* message) {
    // find the end of the line
    int line_length = tok.lineOffset;
    char* temp = tok.start;
    while (*temp != ';' && *temp != '\n' && *temp != '\0') {
        line_length++;
        temp++;
    }
    if (*temp == ';') {
        line_length++;
    }
    fprintf(stderr, "error: %s\n    [main.ship:%i:%i]\n    |\n%03i | %.*s\n    |%*s^^^^ \n",
            message, tok.line, tok.lineOffset, tok.line, line_length,
            tok.start - tok.lineOffset, tok.lineOffset, " ");
}

static void error(Parser* parser, Scanner* scanner, const char* message) {
    print_error(parser->current, message);
    parser->hadError = true;
    synchronize(parser, scanner);
}

static void custom_error(Parser* parser, const char* string, ...) {
    va_list args;
    va_start(args, string);
    vfprintf(stderr, string, args);
    va_end(args);
    parser->hadError = true;
}

static void synchronize(Parser* parser, Scanner* scanner) {
    for (;;) {
        switch (parser->current.type) {
            case TOKEN_EOF:
            case TOKEN_IF:
            case TOKEN_VAR:
            case TOKEN_FN:
            case TOKEN_WHILE:
            case TOKEN_SEMICOLON:
            case TOKEN_LEFT_PAREN:
            case TOKEN_LEFT_BRACE:
            case TOKEN_RIGHT_PAREN:
            case TOKEN_RIGHT_BRACE:
                return;
            default: {
                parser->previous = parser->current;
                parser->current = tokenize(scanner);
            }
        }
    }
}


// ---- token plumbing ---------------------------------------------------

static void init_parser(Parser* parser) {
    parser->hadError = false;
    parser->panicMode = false;
    arena_init(&parser->arena, 1 << 20);
}

static void advance(Scanner* scanner, Parser* parser) {
    // set the parser.previous for the current value
    parser->previous = parser->current;

    // loop until encountered a non error token
    for (;;) {
        parser->current = tokenize(scanner);
        if (parser->current.type != TOKEN_ERROR) return;
        error(parser, scanner, "Unknown token encountered");
    }
}

static void expect(Scanner* scanner, Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(scanner, parser);
        return;
    }
    error(parser, scanner, message);
}


// ---- expressions ------------------------------------------------------

// NOTE: after a syntax error a node can have NULL children. That is fine because
// the compile pass never runs when hadError is set, but it means parse code must
// not dereference the children it just parsed.
static Node* parse_precedence(Parser* parser, Scanner* scanner, Precedence precedence) {
    advance(scanner, parser);
    PrefixFn prefix = get_rule(parser->previous.type)->prefix;
    if (prefix == NULL) {
        error(parser, scanner, "expected expression");
        return NULL;
    }
    Node* left = prefix(parser, scanner);

    while (precedence <= get_rule(parser->current.type)->precedence) {
        advance(scanner, parser);
        InfixFn infix = get_rule(parser->previous.type)->infix;
        left = infix(parser, scanner, left);
    }
    return left;
}

static Node* parse_expression(Parser* parser, Scanner* scanner) {
    return parse_precedence(parser, scanner, PREC_ASSIGNMENT);
}

static Node* parse_number(Parser* parser, Scanner* scanner) {
    (void) scanner;
    Node* n = node_new(&parser->arena, NODE_NUMBER, parser->previous);
    n->as.number = strtod(parser->previous.start, NULL);
    return n;
}

static Node* parse_string(Parser* parser, Scanner* scanner) {
    (void) scanner;
    // the compile pass strips the surrounding quotes
    return node_new(&parser->arena, NODE_STRING, parser->previous);
}

static Node* parse_literal(Parser* parser, Scanner* scanner) {
    (void) scanner;
    return node_new(&parser->arena, NODE_LITERAL, parser->previous);   // true / false / nil
}

static Node* parse_grouping(Parser* parser, Scanner* scanner) {
    Node* inner = parse_precedence(parser, scanner, PREC_OR);
    expect(scanner, parser, TOKEN_RIGHT_PAREN, "unclosed '(' block");
    return inner;                                                      // grouping needs no node
}

static Node* parse_unary(Parser* parser, Scanner* scanner) {
    Node* n = node_new(&parser->arena, NODE_UNARY, parser->previous);
    n->as.child = parse_precedence(parser, scanner, PREC_UNARY);
    return n;
}

// arithmetic AND comparison operators (the compile pass tells them apart)
static Node* parse_binary(Parser* parser, Scanner* scanner, Node* left) {
    Token op = parser->previous;
    Node* n = node_new(&parser->arena, NODE_BINARY, op);
    n->as.binary.left  = left;
    n->as.binary.right = parse_precedence(parser, scanner, (Precedence)(get_rule(op.type)->precedence + 1));
    return n;
}

static Node* parse_identifier(Parser* parser, Scanner* scanner) {
    Token name = parser->previous;
    if (parser->current.type == TOKEN_EQUAL) {
        advance(scanner, parser);                                      // eat '='
        Node* n = node_new(&parser->arena, NODE_ASSIGN, name);
        n->as.child = parse_precedence(parser, scanner, PREC_OR);      // parse the expression value
        return n;
    }
    return node_new(&parser->arena, NODE_VARIABLE, name);
}

static Node* parse_call(Parser* parser, Scanner* scanner, Node* callee) {
    Node* n = node_new(&parser->arena, NODE_CALL, parser->previous);   // the '(' token
    n->as.call.callee = callee;

    while (parser->current.type != TOKEN_EOF && parser->current.type != TOKEN_RIGHT_PAREN) {
        node_list_push(&parser->arena, &n->as.call.args, parse_precedence(parser, scanner, PREC_OR));
        if (parser->current.type != TOKEN_RIGHT_PAREN) {
            expect(scanner, parser, TOKEN_COMMA, "Unexpected token. expected ',' between function arguments");
        }
    }
    if (n->as.call.args.count >= UINT8_MAX) {
        custom_error(parser, "Too much call arguments");
    }
    expect(scanner, parser, TOKEN_RIGHT_PAREN, "Unclosed argument list of a function");
    return n;
}

static Node* parse_array_literal(Parser* parser, Scanner* scanner) {
    Node* n = node_new(&parser->arena, NODE_ARRAY, parser->previous);
    while (parser->current.type != TOKEN_RIGHT_SQUARE_BRACE && parser->current.type != TOKEN_EOF) {
        node_list_push(&parser->arena, &n->as.items, parse_precedence(parser, scanner, PREC_OR));
        if (parser->current.type != TOKEN_RIGHT_SQUARE_BRACE) {
            expect(scanner, parser, TOKEN_COMMA, "Expected , between array values");
        }
    }
    if (n->as.items.count > UINT8_MAX) {
        error(parser, scanner, "Array literal length is too large");
        return n;
    }
    expect(scanner, parser, TOKEN_RIGHT_SQUARE_BRACE, "Unclosed array literal");
    return n;
}

static Node* parse_range_literal(Parser* parser, Scanner* scanner, Node* left) {
    Node* n = node_new(&parser->arena, NODE_RANGE, parser->previous);
    n->as.binary.left  = left;
    n->as.binary.right = parse_precedence(parser, scanner, (Precedence)(PREC_COMPARISON + 1));
    return n;
}

static Node* parse_attribute(Parser* parser, Scanner* scanner, Node* object) {
    if (parser->current.type != TOKEN_IDENTIFIER) {
        error(parser, scanner, "Expected identifier");
        return object;
    }
    advance(scanner, parser);
    Node* n = node_new(&parser->arena, NODE_ATTR, parser->previous);   // token = attribute name
    n->as.child = object;
    return n;
}

static Node* parse_print(Parser* parser, Scanner* scanner) {
    Node* n = node_new(&parser->arena, NODE_PRINT, parser->previous);
    expect(scanner, parser, TOKEN_LEFT_PAREN, "Expected ( after print");
    n->as.child = parse_expression(parser, scanner);
    expect(scanner, parser, TOKEN_RIGHT_PAREN, "Missing parentheses in call");
    return n;
}


// ---- statements -------------------------------------------------------

static void parse_block(Parser* parser, Scanner* scanner, NodeList* body,
                        const char* open_msg, const char* close_msg) {
    expect(scanner, parser, TOKEN_LEFT_BRACE, open_msg);
    while (parser->current.type != TOKEN_RIGHT_BRACE && parser->current.type != TOKEN_EOF) {
        node_list_push(&parser->arena, body, parse_statement(parser, scanner));
    }
    expect(scanner, parser, TOKEN_RIGHT_BRACE, close_msg);
}

// The parse_<keyword> functions below are called AFTER the keyword was consumed,
// so parser->previous is the keyword token.

static Node* parse_if(Parser* parser, Scanner* scanner) {
    Node* n = node_new(&parser->arena, NODE_IF, parser->previous);
    n->as.if_stmt.cond = parse_expression(parser, scanner);
    parse_block(parser, scanner, &n->as.if_stmt.then_body,
                "Expected { after if expression", "Unclosed '}' after block");
    if (parser->current.type == TOKEN_ELSE) {
        advance(scanner, parser);                                      // eat 'else'
        n->as.if_stmt.hasElse = true;
        parse_block(parser, scanner, &n->as.if_stmt.else_body,
                    "Expected '{' after else expression", "Unclosed '}' after block");
    }
    return n;
}

static Node* parse_while(Parser* parser, Scanner* scanner) {
    Node* n = node_new(&parser->arena, NODE_WHILE, parser->previous);
    n->as.while_stmt.cond = parse_expression(parser, scanner);
    parse_block(parser, scanner, &n->as.while_stmt.body,
                "Expected { after while expression", "Expected } after open block");
    return n;
}

static Node* parse_for(Parser* parser, Scanner* scanner) {
    expect(scanner, parser, TOKEN_IDENTIFIER, "expected iter variable to be defined");
    Node* n = node_new(&parser->arena, NODE_FOR, parser->previous);    // token = loop variable
    expect(scanner, parser, TOKEN_IN, "expected in keyword after for loop");
    n->as.for_stmt.iterable = parse_expression(parser, scanner);
    parse_block(parser, scanner, &n->as.for_stmt.body,
                "Expected { after for loop", "Expected } after for loop block");
    return n;
}

static Node* parse_func(Parser* parser, Scanner* scanner) {
    expect(scanner, parser, TOKEN_IDENTIFIER, "Expected identifier");
    Node* n = node_new(&parser->arena, NODE_FUNC, parser->previous);   // token = function name

    expect(scanner, parser, TOKEN_LEFT_PAREN, "Expected ( in function declaration");

    // collect the parameters on the C stack, then copy the exact amount into the arena
    Token tmp[255];
    int count = 0;
    while (parser->current.type != TOKEN_RIGHT_PAREN && parser->current.type != TOKEN_EOF) {
        if (parser->current.type != TOKEN_IDENTIFIER) {
            error(parser, scanner, "Unexpected token");
            break;
        }
        if (count == 255) {
            error(parser, scanner, "Too many parameters");
            break;
        }
        tmp[count++] = parser->current;
        advance(scanner, parser);

        if (parser->current.type == TOKEN_EOF || parser->current.type == TOKEN_RIGHT_PAREN) break;
        expect(scanner, parser, TOKEN_COMMA, "Expected ',' between function arguments");
    }
    if (count > 0) {
        n->as.func.params = (Token*) arena_alloc(&parser->arena, sizeof(Token) * count);
        memcpy(n->as.func.params, tmp, sizeof(Token) * count);
    }
    n->as.func.paramCount = count;

    expect(scanner, parser, TOKEN_RIGHT_PAREN, "Unclosed ) in function declaration");
    parse_block(parser, scanner, &n->as.func.body,
                "Expected open block in function declaration", "Unclosed block in function declaration");
    return n;
}

static Node* parse_var(Parser* parser, Scanner* scanner) {
    // var x = 5;
    expect(scanner, parser, TOKEN_IDENTIFIER, "Expected variable name after var");
    Node* n = node_new(&parser->arena, NODE_VAR, parser->previous);
    expect(scanner, parser, TOKEN_EQUAL, "Expected '=' after variable declaration");
    n->as.child = parse_precedence(parser, scanner, PREC_OR);
    expect(scanner, parser, TOKEN_SEMICOLON, "Expected ;");
    return n;
}

static Node* parse_global(Parser* parser, Scanner* scanner) {
    // glob x = 5;
    expect(scanner, parser, TOKEN_IDENTIFIER, "Expected variable name after glob");
    Node* n = node_new(&parser->arena, NODE_GLOBAL, parser->previous);
    expect(scanner, parser, TOKEN_EQUAL, "Expected '=' after global assignment");
    n->as.child = parse_precedence(parser, scanner, PREC_OR);
    expect(scanner, parser, TOKEN_SEMICOLON, "Expected ;");
    return n;
}

static Node* parse_return(Parser* parser, Scanner* scanner) {
    Node* n = node_new(&parser->arena, NODE_RETURN, parser->previous);
    if (parser->current.type != TOKEN_SEMICOLON) {
        n->as.child = parse_precedence(parser, scanner, PREC_OR);      // stays NULL for a bare `return;`
    }
    expect(scanner, parser, TOKEN_SEMICOLON, "Expected ;");
    return n;
}

static Node* parse_statement(Parser* parser, Scanner* scanner) {
    switch (parser->current.type) {
        case TOKEN_IF:     advance(scanner, parser); return parse_if(parser, scanner);
        case TOKEN_WHILE:  advance(scanner, parser); return parse_while(parser, scanner);
        case TOKEN_FOR:    advance(scanner, parser); return parse_for(parser, scanner);
        case TOKEN_FN:     advance(scanner, parser); return parse_func(parser, scanner);
        case TOKEN_VAR:    advance(scanner, parser); return parse_var(parser, scanner);
        case TOKEN_RETURN: advance(scanner, parser); return parse_return(parser, scanner);
        case TOKEN_GLOBAL:   advance(scanner, parser); return parse_global(parser, scanner);
        default: {
            // statements that only evaluate an expression, for ex: call(a,b,c);
            Node* n = node_new(&parser->arena, NODE_EXPR_STMT, parser->current);
            n->as.child = parse_expression(parser, scanner);
            expect(scanner, parser, TOKEN_SEMICOLON, "Expected ;");
            return n;
        }
    }
}


// Statement keywords (fn, for, if, return, var, while, glob) are handled by
// parse_statement and deliberately have no entry here: they are not expressions.
static ParseRule rules[] = {
        [TOKEN_LEFT_PAREN]         = {parse_grouping,      parse_call,          PREC_CALL},
        [TOKEN_RIGHT_PAREN]        = {NULL,                NULL,                PREC_NONE},
        [TOKEN_LEFT_BRACE]         = {NULL,                NULL,                PREC_NONE},
        [TOKEN_RIGHT_BRACE]        = {NULL,                NULL,                PREC_NONE},
        [TOKEN_LEFT_SQUARE_BRACE]  = {parse_array_literal, NULL,                PREC_NONE},
        [TOKEN_DOT_DOT]            = {NULL,                parse_range_literal, PREC_CALL},
        [TOKEN_RIGHT_SQUARE_BRACE] = {NULL,                NULL,                PREC_NONE},
        [TOKEN_COMMA]              = {NULL,                NULL,                PREC_NONE},
        [TOKEN_DOT]                = {NULL,                parse_attribute,     PREC_CALL},
        [TOKEN_MINUS]              = {parse_unary,         parse_binary,        PREC_TERM},
        [TOKEN_PLUS]               = {NULL,                parse_binary,        PREC_TERM},
        [TOKEN_SEMICOLON]          = {NULL,                NULL,                PREC_NONE},
        [TOKEN_SLASH]              = {NULL,                parse_binary,        PREC_FACTOR},
        [TOKEN_MODULO]             = {NULL,                parse_binary,        PREC_FACTOR},
        [TOKEN_STAR]               = {NULL,                parse_binary,        PREC_FACTOR},
        [TOKEN_BANG]               = {parse_unary,         NULL,                PREC_NONE},
        [TOKEN_BANG_EQUAL]         = {NULL,                parse_binary,        PREC_EQUALITY},
        [TOKEN_EQUAL]              = {NULL,                NULL,                PREC_NONE},
        [TOKEN_EQUAL_EQUAL]        = {NULL,                parse_binary,        PREC_EQUALITY},
        [TOKEN_GREATER]            = {NULL,                parse_binary,        PREC_EQUALITY},
        [TOKEN_GREATER_EQUAL]      = {NULL,                parse_binary,        PREC_EQUALITY},
        [TOKEN_LESS]               = {NULL,                parse_binary,        PREC_EQUALITY},
        [TOKEN_LESS_EQUAL]         = {NULL,                parse_binary,        PREC_EQUALITY},
        [TOKEN_IDENTIFIER]         = {parse_identifier,    NULL,                PREC_NONE},
        [TOKEN_STRING]             = {parse_string,        NULL,                PREC_NONE},
        [TOKEN_NUMBER]             = {parse_number,        NULL,                PREC_NONE},
        [TOKEN_ELSE]               = {NULL,                NULL,                PREC_NONE},
        [TOKEN_FALSE]              = {parse_literal,       NULL,                PREC_NONE},
        [TOKEN_IN]                 = {NULL,                NULL,                PREC_NONE},
        [TOKEN_VERTICAL_BAR]       = {NULL,                NULL,                PREC_NONE},
        [TOKEN_NIL]                = {parse_literal,       NULL,                PREC_NONE},
        [TOKEN_PRINT]              = {parse_print,         NULL,                PREC_NONE},
        [TOKEN_SUPER]              = {NULL,                NULL,                PREC_NONE},
        [TOKEN_THIS]               = {NULL,                NULL,                PREC_NONE},
        [TOKEN_TRUE]               = {parse_literal,       NULL,                PREC_NONE},
        [TOKEN_ERROR]              = {NULL,                NULL,                PREC_NONE},
        [TOKEN_EOF]                = {NULL,                NULL,                PREC_NONE},
};

static ParseRule* get_rule(uint8_t token) {
    return &rules[token];
}


typedef struct FunctionCtx {
    FunctionObj* func;
    HashMap*     varMap;
    struct FunctionCtx* enclosing;
} FunctionCtx;

typedef struct {
    FunctionCtx* ctx;      // innermost function being compiled
    bool hadError;
} Compiler;

static Chunk* current_chunk(Compiler* c) {
    return &c->ctx->func->body;
}

static void emit(Compiler* c, uint8_t op, int line) {
    write_chunk(current_chunk(c), op, line);
}

static void emit_bytes(Compiler* c, uint8_t a, uint8_t b, int line) {
    write_bytes(current_chunk(c), a, b, line);
}

// constant-pool entry holding the name of an identifier token
static uint8_t name_constant(Compiler* c, Token t) {
    return add_constant(current_chunk(c), VAR_OBJ(create_string_obj(t.start, t.length)));
}

static void compile_error(Compiler* c, Token at, const char* message) {
    print_error(at, message);
    c->hadError = true;
}


// ---- locals -----------------------------------------------------------

static HashNode* get_variable(Compiler* c, char* name, int length) {
    return get_node(c->ctx->varMap, name, length);
}

static unsigned int create_variable(Compiler* c, char* name, int length) {
    Local local;
    local.name = name;
    local.value = VAR_NIL;
    local.length = length;
    c->ctx->func->locals[c->ctx->varMap->count] = local;

    put_node(c->ctx->varMap, name, length, c->ctx->varMap->count);
    return c->ctx->varMap->count - 1;
}

static unsigned int add_variable(Compiler* c, char* name, int length) {
    HashNode* existing = get_variable(c, name, length);
    if (existing != NULL) {
        return existing->value;
    }
    return create_variable(c, name, length);
}


// ---- jumps ------------------------------------------------------------

// Emits `op` with a 2-byte placeholder operand; returns the offset of that operand.
static int emit_jump(Compiler* c, uint8_t op, int line) {
    emit(c, op, line);
    int operand = current_chunk(c)->count;
    emit_bytes(c, 0xff, 0xff, line);
    return operand;
}

static void patch_jump(Compiler* c, int operand, int size, Token at) {
    if (size > UINT16_MAX) {
        compile_error(c, at, "max jump length exceeded");
    }
    current_chunk(c)->codes[operand]     = (size >> 8) & 0xff;
    current_chunk(c)->codes[operand + 1] = size & 0xff;
}


// ---- nodes ------------------------------------------------------------

static void compile_node(Compiler* c, Node* n);

static void compile_body(Compiler* c, NodeList* body) {
    for (int i = 0; i < body->count; i++) {
        compile_node(c, body->items[i]);
    }
}

static void compile_binary(Compiler* c, Node* n) {
    int line = n->token.line;
    compile_node(c, n->as.binary.left);
    compile_node(c, n->as.binary.right);

    switch (n->token.type) {
        case TOKEN_PLUS:          emit(c, OP_ADD, line); break;
        case TOKEN_MINUS:         emit(c, OP_SUB, line); break;
        case TOKEN_STAR:          emit(c, OP_MUL, line); break;
        case TOKEN_SLASH:         emit(c, OP_DIV, line); break;
        case TOKEN_MODULO:        emit(c, OP_MODULO, line); break;
        case TOKEN_EQUAL_EQUAL:   emit(c, OP_COMPARE, line); break;
        case TOKEN_BANG_EQUAL:    emit(c, OP_COMPARE, line);      emit(c, OP_NOT, line); break;
        case TOKEN_LESS:          emit(c, OP_LESS_THAN, line); break;
        case TOKEN_LESS_EQUAL:    emit(c, OP_GREATER_THAN, line); emit(c, OP_NOT, line); break;
        case TOKEN_GREATER:       emit(c, OP_GREATER_THAN, line); break;
        case TOKEN_GREATER_EQUAL: emit(c, OP_LESS_THAN, line);    emit(c, OP_NOT, line); break;
        default: compile_error(c, n->token, "Unexpected Binary Token");   // unreachable
    }
}

static void compile_if(Compiler* c, Node* n) {
    int line = n->token.line;
    compile_node(c, n->as.if_stmt.cond);
    int then_operand = emit_jump(c, OP_POP_JUMP_IF_FALSE, line);

    compile_body(c, &n->as.if_stmt.then_body);

    // calculate the size of the body, and patch the jump
    int body_size = current_chunk(c)->count - then_operand - 2;
    if (!n->as.if_stmt.hasElse) {
        patch_jump(c, then_operand, body_size, n->token);
        return;
    }
    // +3: the false branch must also skip the OP_JUMP emitted right below
    patch_jump(c, then_operand, body_size + 3, n->token);

    int else_operand = emit_jump(c, OP_JUMP, line);
    compile_body(c, &n->as.if_stmt.else_body);
    patch_jump(c, else_operand, current_chunk(c)->count - else_operand - 2, n->token);
}

static void compile_while(Compiler* c, Node* n) {
    int line = n->token.line;
    int before_bool = current_chunk(c)->count;

    compile_node(c, n->as.while_stmt.cond);
    int exit_operand = emit_jump(c, OP_POP_JUMP_IF_FALSE, line);

    compile_body(c, &n->as.while_stmt.body);

    // +1: the exit jump must also skip the 3-byte OP_JUMP_BACKWARD (operand bytes already counted)
    int body_size = current_chunk(c)->count - exit_operand + 1;
    patch_jump(c, exit_operand, body_size, n->token);

    emit(c, OP_JUMP_BACKWARD, line);
    int total = current_chunk(c)->count + 2 - before_bool;   // +2 for the operand bytes emitted next
    if (total > UINT16_MAX) {
        compile_error(c, n->token, "max jump length exceeded");
    }
    emit_bytes(c, (total >> 8) & 0xff, total & 0xff, line);
}

static void compile_for(Compiler* c, Node* n) {
    int line = n->token.line;
    compile_node(c, n->as.for_stmt.iterable);
    emit(c, OP_GET_ITER, line);

    int for_iter_loc = current_chunk(c)->count;
    emit(c, OP_FOR_ITER, line);
    emit_bytes(c, 0xff, 0xff, line);

    unsigned int var_index = add_variable(c, n->token.start, n->token.length);
    emit_bytes(c, OP_STORE_FAST, var_index, line);

    compile_body(c, &n->as.for_stmt.body);

    emit(c, OP_JUMP_BACKWARD, line);
    int body_size = current_chunk(c)->count - for_iter_loc + 2;
    if (body_size > UINT16_MAX) {
        compile_error(c, n->token, "max jump length exceeded");
    }
    emit_bytes(c, (body_size >> 8) & 0xff, body_size & 0xff, line);

    // set the jump over the loop (lands right after the OP_JUMP_BACKWARD instruction)
    int jmp_over = body_size - 3;
    current_chunk(c)->codes[for_iter_loc + 1] = (jmp_over >> 8) & 0xff;
    current_chunk(c)->codes[for_iter_loc + 2] = jmp_over & 0xff;
    emit(c, OP_END_FOR, line);
}

static void compile_func(Compiler* c, Node* n) {
    int line = n->token.line;

    // create the function's context and make it the current one
    FunctionCtx ctx;
    ctx.func      = create_func_obj(n->token.start, n->token.length, FN_FUNCTION);
    ctx.varMap    = (HashMap*) malloc(sizeof(HashMap));
    ctx.enclosing = c->ctx;
    create_variable_map(ctx.varMap);
    c->ctx = &ctx;

    // arguments are the first locals
    for (int i = 0; i < n->as.func.paramCount; i++) {
        add_variable(c, n->as.func.params[i].start, n->as.func.params[i].length);
    }

    compile_body(c, &n->as.func.body);
    emit_bytes(c, OP_NIL, OP_RETURN, line);

    ctx.func->localCount = ctx.varMap->count;
    // Free the hashmap AFTER assigning the right localCount.
    free_hash_map(ctx.varMap);
    c->ctx = ctx.enclosing;

    // now current_chunk() is the ENCLOSING function's chunk: add the function constant
    uint8_t index = add_constant(current_chunk(c), VAR_OBJ(ctx.func));
    emit_bytes(c, OP_CONSTANT, index, line);

    // register the function name
    unsigned int slot = add_variable(c, n->token.start, n->token.length);
    emit_bytes(c, OP_STORE_FAST, slot, line);
}

static void compile_node(Compiler* c, Node* n) {
    if (n == NULL) return;
    int line = n->token.line;

    switch (n->type) {
        case NODE_NUMBER: {
            uint8_t index = add_constant(current_chunk(c), VAR_NUMBER(n->as.number));
            emit_bytes(c, OP_CONSTANT, index, line);
            break;
        }
        case NODE_STRING: {
            // strip the surrounding quotes
            StringObj* obj = create_string_obj(n->token.start + 1, n->token.length - 2);
            uint8_t index = add_constant(current_chunk(c), VAR_OBJ(obj));
            emit_bytes(c, OP_CONSTANT, index, line);
            break;
        }
        case NODE_LITERAL:
            switch (n->token.type) {
                case TOKEN_FALSE: emit(c, OP_FALSE, line); break;
                case TOKEN_TRUE:  emit(c, OP_TRUE, line);  break;
                case TOKEN_NIL:   emit(c, OP_NIL, line);   break;
                default: break;   // unreachable
            }
            break;
        case NODE_UNARY:
            compile_node(c, n->as.child);
            if (n->token.type == TOKEN_MINUS)     emit(c, OP_NEGATE, line);
            else if (n->token.type == TOKEN_BANG) emit(c, OP_NOT, line);
            break;
        case NODE_BINARY:
            compile_binary(c, n);
            break;

        case NODE_VARIABLE: {
            HashNode* var = get_variable(c, n->token.start, n->token.length);
            if (var == NULL) {
                emit_bytes(c, OP_LOAD_GLOBAL, name_constant(c, n->token), line);
            } else {
                emit_bytes(c, OP_LOAD_LOCAL, var->value, line);
            }
            break;
        }
        case NODE_ASSIGN: {
            emit(c, OP_NIL, line);   // the assignment expression evaluates to nil
            HashNode* var = get_variable(c, n->token.start, n->token.length);
            if (var == NULL) {
                char msg[256];
                snprintf(msg, sizeof msg,
                         "variable '%.*s' is not defined in the current scope. did you mean 'glob %.*s = ...'",
                         n->token.length, n->token.start, n->token.length, n->token.start);
                compile_error(c, n->token, msg);
                break;
            }
            compile_node(c, n->as.child);
            emit_bytes(c, OP_ASSIGN_LOCAL, var->value, line);
            break;
        }
        case NODE_CALL:
            compile_node(c, n->as.call.callee);
            for (int i = 0; i < n->as.call.args.count; i++) {
                compile_node(c, n->as.call.args.items[i]);
            }
            emit_bytes(c, OP_CALL, n->as.call.args.count, line);
            break;
        case NODE_ATTR:
            compile_node(c, n->as.child);
            emit_bytes(c, OP_LOAD_ATTR, name_constant(c, n->token), line);
            break;
        case NODE_ARRAY:
            compile_body(c, &n->as.items);
            emit_bytes(c, OP_BUILD_ARRAY, n->as.items.count, line);
            break;
        case NODE_RANGE:
            compile_node(c, n->as.binary.left);
            compile_node(c, n->as.binary.right);
            emit(c, OP_BUILD_RANGE, line);
            break;
        case NODE_PRINT:
            emit(c, OP_NIL, line);   // push nil as print doesn't return anything
            compile_node(c, n->as.child);
            emit(c, OP_SHOW_TOP, line);
            break;

        case NODE_EXPR_STMT:
            compile_node(c, n->as.child);
            emit(c, OP_POP_TOP, line);
            break;
        case NODE_VAR: {
            // check if variable is already defined in the current scope
            if (get_variable(c, n->token.start, n->token.length) != NULL) {
                compile_error(c, n->token, "redeclaration of variable");
                break;
            }
            // value first: in `var x = x + 1` the right-hand x is NOT the new variable
            compile_node(c, n->as.child);
            unsigned int index = create_variable(c, n->token.start, n->token.length);
            emit_bytes(c, OP_STORE_FAST, index, line);
            break;
        }
        case NODE_GLOBAL:
            compile_node(c, n->as.child);
            emit_bytes(c, OP_ASSIGN_GLOBAL, name_constant(c, n->token), line);
            break;
        case NODE_RETURN:
            if (c->ctx->func->type == FN_SCRIPT) {
                compile_error(c, n->token, "Return keyword outside of function");
            }
            if (n->as.child != NULL) {
                compile_node(c, n->as.child);
            } else {
                emit(c, OP_NIL, line);   // no expression after the return: return nil
            }
            emit(c, OP_RETURN, line);
            break;

        case NODE_IF:    compile_if(c, n);    break;
        case NODE_WHILE: compile_while(c, n); break;
        case NODE_FOR:   compile_for(c, n);   break;
        case NODE_FUNC:  compile_func(c, n);  break;
    }
}


FunctionObj* compile(const char* source) {
    Scanner scanner = create_token_scanner(source);
    Parser parser;
    init_parser(&parser);

    advance(&scanner, &parser);
    NodeList program = {0};
    while (parser.current.type != TOKEN_EOF) {
        node_list_push(&parser.arena, &program, parse_statement(&parser, &scanner));
    }
    if (parser.hadError) {
        arena_free(&parser.arena);
        return NULL;
    }

    Compiler c = { .ctx = NULL, .hadError = false };
    FunctionCtx script;
    script.func      = create_func_obj("main", 4, FN_SCRIPT);
    script.varMap    = (HashMap*) malloc(sizeof(HashMap));
    script.enclosing = NULL;
    create_variable_map(script.varMap);
    c.ctx = &script;

    compile_body(&c, &program);
    emit(&c, OP_HALT, scanner.line);

    script.func->localCount = script.varMap->count;
    free_hash_map(script.varMap);
    arena_free(&parser.arena);

    return c.hadError ? NULL : script.func;
}