#include "compiler.h"
#include "token.h"
#include  "memory.h"

#include <stdio.h>
#include <stdlib.h>

// Stack for the 'shunting yard algorithm'
typedef struct {
	int count;
	int capacity;
	int* values;
} ExpressionStack;

// stack related functions
static void init_expression_stack(ExpressionStack* stack) {
	stack->count = 0;
	stack->capacity = 0;
	stack->values = NULL;
}

static void free_expression_stack(ExpressionStack* stack) {
	FREE_ARRAY(int, stack->values, stack->capacity);
	init_expression_stack(stack);
}

static void push_expression(ExpressionStack* stack, int value) {
	if (stack->capacity < stack->count + 1) {
		int oldCapacity = stack->capacity;
		stack->capacity = GROW_CAPACITY(oldCapacity);
		stack->values = GROW_ARRAY(int, stack->values, oldCapacity, stack->capacity);
	}
	stack->values[stack->count] = value;
	stack->count++;
}

static int pop_expression(ExpressionStack* stack) {
	if (stack->count == 0) {
		printf("Expression stack is empty");
		exit(1);
	}
	return stack->values[stack->count--];
}

static int peek_expression(ExpressionStack* stack) {
	if (stack->count == 0) {
		printf("Expression stack is empty");
		exit(1);
	}
	return stack->values[stack->count - 1];
}

static int peek_next_expression(ExpressionStack* stack) {
	if (stack->count <= 1) {
		printf("Expression stack is empty");
		exit(1);
	}
	return stack->values[stack->count - 2];
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

static void parse_number(Parser* parser, Scanner* scanner) {
	double value = strtod(parser->current.start, NULL);
	uint8_t index = add_constant(parser->currentChunk, NUMBER(value));
	write_bytes(parser->currentChunk, OP_CONSTANT, index);
}

static void parse_binary(Parser* parser, Scanner* scanner) {
	TokenType prev = parser->current.type;
	advance(scanner, parser);
	parse_expression(parser, scanner);
	switch (prev) {
	case TOKEN_PLUS: write_chunk(parser->currentChunk, OP_ADD); break;
	}
}

static void parse_unary(Parser* parser, Scanner* scanner) {
	// currently the only unary operator that supported is -
	TokenType type = parser->previous.type;
	parse_expression(parser, scanner);

	switch (parser->previous.type) {
	case TOKEN_MINUS: write_chunk(parser->currentChunk, OP_NEGATE); break;
	}
}

static void parse_expression(Parser* parser, Scanner* scanner) {
	switch (parser->current.type) {
	case TOKEN_MINUS: parse_unary(parser, scanner); break;
	case TOKEN_NUMBER: parse_number(parser, scanner); break;
	case TOKEN_PLUS: parse_binary(parser, scanner); break;
	default: return;
	}
	switch (parser->current.type) {
	case TOKEN_MINUS: parse_unary(parser, scanner); break;
	case TOKEN_NUMBER: parse_number(parser, scanner); break;
	case TOKEN_PLUS: parse_binary(parser, scanner); break;
	default: return;
	}
}

static void end_compile(Parser* parser, Scanner* scanner) {
	advance(scanner, parser);
	if (parser->current.type == TOKEN_EOF) {
		write_chunk(parser->currentChunk, OP_HALT);
		free_expression_stack(&parser->expressionStack);
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