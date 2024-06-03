#pragma once
#ifndef SHIP_TOKEN_H_
#define SHIP_TOKEN_H_

typedef enum {
	// Single-character tokens.
	TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_VERTICAL_BAR,
	TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
	TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS, TOKEN_MODULO,
	TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,
	// One or two character tokens.
	TOKEN_BANG, TOKEN_BANG_EQUAL,
	TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
	TOKEN_GREATER, TOKEN_GREATER_EQUAL,
	TOKEN_LESS, TOKEN_LESS_EQUAL,
	// Literals.
	TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
	// Keywords.
	TOKEN_ELSE, TOKEN_FALSE,
	TOKEN_FOR, TOKEN_FN, TOKEN_IF, TOKEN_NIL,
    TOKEN_FOREACH,
	TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
	TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE, TOKEN_GLOBAL,

	TOKEN_ERROR, TOKEN_EOF
} TokenType;

typedef struct {
	TokenType type;
	char* start;
	int length;
	int line;
    int lineOffset;
} Token;

typedef struct {
	char* start;
	char* current;
	int line;
    int lineOffset;
} Scanner;

Token tokenize(Scanner *scanner);
Scanner create_token_scanner(const char* source);

#endif // !SHIP_TOKEN_H_
