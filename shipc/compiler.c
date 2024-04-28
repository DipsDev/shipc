#include "compiler.h"
#include "token.h"
#include  "memory.h"

#include <stdio.h>
#include <stdlib.h>


// Stack for the 'shunting yard algorithm'
typedef struct {
	int count;
	int capacity;
	TokenType* values;
} ExpressionStack;

// stack related functions
static void init_expression_stack(ExpressionStack* stack) {
	stack->count = 0;
	stack->capacity = 0;
	stack->values = NULL;
}

static void free_expression_stack(ExpressionStack* stack) {
	FREE_ARRAY(TokenType, stack->values, stack->capacity);
	init_expression_stack(stack);
}

static void push_expression(ExpressionStack* stack, TokenType value) {
	if (stack->capacity <= stack->count + 1) {
		int oldCapacity = stack->capacity;
		stack->capacity = GROW_CAPACITY(oldCapacity);
		stack->values = GROW_ARRAY(TokenType, stack->values, oldCapacity, stack->capacity);
	}
	stack->values[stack->count] = value;
	stack->count++;
}

static is_empty(ExpressionStack* stack) {
	return stack->count == 0;
}

static TokenType pop_expression(ExpressionStack* stack) {
	if (stack->count == 0) {
		printf("Expression stack is empty");
		exit(1);
	}
	stack->count--;
	return (TokenType)stack->values[stack->count];
}

static TokenType peek_expression(ExpressionStack* stack) {
	if (stack->count == 0) {
		printf("Expression stack is empty");
		exit(1);
	}
	return stack->values[stack->count - 1];
}

//////


// main parser struct
typedef struct {
	Token current;
	Token previous;

	Chunk* currentChunk;

	bool hadError;
	bool panicMode;

	ExpressionStack expressionStack;
} Parser;

// declare functions
static void parse_expression(Parser* parser, Scanner* scanner);
static void parse_binary(Parser* parser, Scanner* scanner, TokenType type);
static void parse_unary(Parser* parser, Scanner* scanner, TokenType type);
static void parse_number(Parser* parser, Scanner* scanner, TokenType type);


/// precedence
typedef enum {
	PREC_NONE, // none
	PREC_TERM, // + / -
	PREC_FACTOR, // * / /
	PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(Parser* parser, Scanner* scanner, TokenType type);
struct ParseRule {
	ParseFn prefix;
	ParseFn infix;
	// bool leftAssociative
	Precedence precedence;
};

struct ParseRule rules[] = {
	[TOKEN_NUMBER] = {NULL, parse_number, PREC_NONE},
	[TOKEN_PLUS] = {NULL, parse_binary, PREC_TERM},
	[TOKEN_MINUS] = {parse_unary, parse_binary, PREC_TERM},
	[TOKEN_SLASH] = {NULL, parse_binary, PREC_FACTOR},
	[TOKEN_STAR] = {NULL, parse_binary,PREC_FACTOR},
	[TOKEN_EOF] = {NULL, NULL,PREC_NONE},
};

static struct ParseRule* get_rule(TokenType type) {
	return &rules[type];
}

///// 




// error related functions
static void error(Parser* parser, const char* string) {
	fprintf(stderr, "[ERROR] %s '%.*s'\n", string, parser->current.length, parser->current.start);
	parser->hadError = true;
}

static void errorAtCurrent(Parser* parser) {
	error(parser, "encountered an error");
	parser->hadError = true;
}



static void init_parser(Parser* parser, Chunk* chunk) {
	parser->hadError = false;
	parser->panicMode = false;

	parser->currentChunk = chunk;
	ExpressionStack stack;
	init_expression_stack(&stack);
	parser->expressionStack = stack;
}

static void advance(Scanner* scanner, Parser* parser) {
	// set the parser.previous for the current value
	parser->previous = parser->current;

	// loop until encountered a non error token
	for (;;) {
		parser->current = tokenize(scanner);
		if (parser->current.type != TOKEN_ERROR) return;

		errorAtCurrent(parser);
	}
}

static void parse_number(Parser* parser, Scanner* scanner, TokenType type) {
	double value = strtod(parser->previous.start, NULL);
	uint8_t index = add_constant(parser->currentChunk, VAR_NUMBER(value));
	write_bytes(parser->currentChunk, OP_CONSTANT, index);
}

static void parse_binary(Parser* parser, Scanner* scanner, TokenType type) {
	switch (type) {
	case TOKEN_PLUS: write_chunk(parser->currentChunk, OP_ADD); break;
	case TOKEN_MINUS: write_chunk(parser->currentChunk, OP_SUB); break;
	case TOKEN_STAR: write_chunk(parser->currentChunk, OP_MUL); break;
	case TOKEN_SLASH: write_chunk(parser->currentChunk, OP_DIV); break;
	}
}

static void parse_unary(Parser* parser, Scanner* scanner, TokenType type) {
	// currently the only unary operator that supported is -
	// parse_expression(parser, scanner);

	switch (type) {
	case TOKEN_MINUS: write_chunk(parser->currentChunk, OP_NEGATE); break;
	}
}

static void parse_precedence(Parser* parser, Scanner* scanner) {
	struct ParseRule* curr = get_rule(parser->previous.type);
	// add more predenet operators
	while (!is_empty(&parser->expressionStack) && curr->precedence <= get_rule(peek_expression(&parser->expressionStack))->precedence) {
		TokenType type = pop_expression(&parser->expressionStack);
		get_rule(type)->infix(parser, scanner, type);
	}
	push_expression(&parser->expressionStack, parser->previous.type);
}

static void parse_expression(Parser* parser, Scanner* scanner) {
	for (;;) {
		advance(scanner, parser);
		if (parser->previous.type == TOKEN_EOF) break;
		// if token is a number, add it to the bytecode
		if (parser->previous.type == TOKEN_NUMBER) {
			parse_number(parser, scanner, parser->previous.type);
			continue;
		}
		// parse according to precedence
		parse_precedence(parser, scanner);
	}
	// clean the stack;
	while (!is_empty(&parser->expressionStack)) {
		TokenType type = pop_expression(&parser->expressionStack);
		get_rule(type)->infix(parser, scanner, type);
	}
}

static void end_compile(Parser* parser, Scanner* scanner) {
	free_expression_stack(&parser->expressionStack);
	if (parser->current.type == TOKEN_EOF) {
		write_chunk(parser->currentChunk, OP_HALT);
		return;
	}
	error(parser, "expected EOF at end of file got");
}

bool compile(const char* source, Chunk* chunk) {
	// create objects
	Scanner scanner = create_token_scanner(source);
	Parser parser;
	init_parser(&parser, chunk); // inits the parser
	advance(&scanner, &parser); // make sure that parser.current is a valid token
	parse_expression(&parser, &scanner);
	end_compile(&parser, &scanner); // end compilation



	return !parser.hadError;
}