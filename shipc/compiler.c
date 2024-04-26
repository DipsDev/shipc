#include "compiler.h"
#include "token.h"

#include <stdio.h>
#include <stdlib.h>


typedef struct {
	Token current;
	Token previous;

	Chunk* currentChunk;

	bool hadError;
	bool panicMode;
} Parser;

// declare functions
static void parse_expression(Parser* parser, Scanner* scanner);

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

		fprintf(stderr, "Error encountered");
		parser->hadError = true;
	}
}

static void parse_number(Parser* parser, Scanner* scanner) {
	double value = strtod(parser->current.start, NULL);
	uint8_t index = add_constant(parser->currentChunk, NUMBER(value));
	write_bytes(parser->currentChunk, OP_CONSTANT, index);
}

static void parse_unary(Parser* parser, Scanner* scanner) {
	// advance so previous has a value
	advance(scanner, parser);
	// currently the only unary operator that supported is -
	parse_expression(parser, scanner);
	switch (parser->previous.type) {
	case TOKEN_MINUS: write_chunk(parser->currentChunk, OP_NEGATE); break;
	}
}

static void parse_expression(Parser* parser, Scanner* scanner) {
	switch (parser->current.type) {
	case TOKEN_MINUS: parse_unary(parser, scanner); break;
	case TOKEN_NUMBER: parse_number(parser, scanner); break;
	default: return;
	}
}

static void end_compile(Parser* parser, Scanner* scanner) {
	advance(scanner, parser);
	if (parser->current.type == TOKEN_EOF) {
		write_chunk(parser->currentChunk, OP_HALT);
		return;
	}
	printf("Error");
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