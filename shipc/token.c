#include <stdbool.h>
#include <stdio.h>

#include "token.h"

typedef struct {
	const char* start;
	const char* current;
	int line;
} Scanner;

static char advance(Scanner* scanner) {
	(scanner->current)++;
	return *(scanner->current - 1);
}

static bool isAtEnd(Scanner* scanner) {
	return *(scanner->current) == '\0';
}

static char peek(Scanner* scanner) {
	return *(scanner->current);
}

static char peek_next(Scanner* scanner) {
	return *((scanner->current + 1));
}

static Token create_token(Scanner *scanner, TokenType type) {
	Token tkn;
	tkn.length = (int) (scanner->current - scanner->start);
	tkn.start = scanner->start;
	tkn.type = type;
	return tkn;
}

static Token create_error_token(Scanner *scanner, const char* error_message) {
	Token tkn;
	tkn.length = strlen(error_message);
	tkn.start = error_message;
	tkn.type = TOKEN_ERROR;
	return tkn;
}

static Scanner create_token_scanner(const char* source) {
	Scanner scan;
	scan.current = source;
	scan.start = source;
	scan.line = 0;
	return scan;
}


static Token string(Scanner* scanner) {
	while (!isAtEnd(scanner) && peek(scanner) != '"') {
		advance(scanner);
	}
	if (isAtEnd(scanner)) {
		return create_error_token(scanner, "EOL while scanning string literal");
	}
	advance(scanner); // eat the ".
	return create_token(scanner, TOKEN_STRING);
}

static bool is_numeric(char c) {
	return c <= '9' && c >= '0';
}

static bool is_alpha(char c) {
	return (c >= 'a' && c <= 'z') || c == '_' || (c >= 'A' && c <= 'Z');
}

static Token number(Scanner* scanner) {
	while (is_numeric(peek(scanner))) advance(scanner);
	if (peek(scanner) == '.' && is_numeric(peek_next(scanner))) {
		advance(scanner);
		while (is_numeric(peek(scanner))) advance(scanner);
	}

	return create_token(scanner, TOKEN_NUMBER);
}

static Token identifier(Scanner* scanner) {
	while (is_alpha(peek(scanner)) || is_numeric(peek(scanner))) advance(scanner);

	return create_token(scanner, TOKEN_IDENTIFIER);
}

static void remove_whitespaces(Scanner *scanner) {
	for (;;) {
		char c = peek(scanner);
		switch (c) {
			case ' ':
			case '\t':
			case '\r': advance(scanner); break;
			default: {
				return;
			}
		}
	}
}


static Token scan_token(Scanner *scanner) {
	scanner->start = scanner->current;
	char c = advance(scanner);
	switch (c) {
		case '+': create_token(scanner, TOKEN_PLUS); break;
		case '*': create_token(scanner, TOKEN_STAR); break;
		case '/': create_token(scanner, TOKEN_SLASH); break;
		case '-': create_token(scanner, TOKEN_MINUS); break;
		case '{': create_token(scanner, TOKEN_LEFT_BRACE); break;
		case '}': create_token(scanner, TOKEN_RIGHT_BRACE); break;
		case '(': create_token(scanner, TOKEN_LEFT_PAREN); break;
		case ')': create_token(scanner, TOKEN_RIGHT_PAREN); break;
		case '.': create_token(scanner, TOKEN_DOT); break;
		case ',': create_token(scanner, TOKEN_COMMA); break;
		case ';': create_token(scanner, TOKEN_SEMICOLON); break;
		case '"': string(scanner); break;
		default: {
			if (is_numeric(c)) {
				return number(scanner);
			}
			if (is_alpha(c)) {
				return identifier(scanner);
			}
			return create_error_token(scanner, "Unknown Token");
		}
	}
}

void tokenize(const char* source) {
	Scanner scanner = create_token_scanner(source);
	for (;;) {
		remove_whitespaces(&scanner);
		if (isAtEnd(&scanner)) {
			return;
		}
		Token token = scan_token(&scanner);
		printf("%2d '%.*s'\n", token.type, token.length, token.start);
	};
}


