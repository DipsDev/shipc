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

static void init_parser(Parser* parser, Chunk* chunk) {
	parser->hadError = false;
	parser->panicMode = false;

	parser->currentChunk = chunk;
}



bool compile(const char* source, Chunk* chunk) {
	// create objects
	Scanner scanner = create_token_scanner(source);
	Parser parser;
	init_parser(&parser, chunk);

	advance(&scanner, &parser);

	consume(&parser, &scanner, TOKEN_EOF);

	return !parser.hadError;
}