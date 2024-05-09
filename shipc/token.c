#include <stdbool.h>
#include <stdio.h>

#include "token.h"

static char advance(Scanner* scanner) {
	(scanner->current)++;
	return *(scanner->current - 1);
}

static char peek(Scanner* scanner) {
	return *(scanner->current);
}

static bool match(Scanner* scanner, char expected) {
	if (peek(scanner) == expected) {
		advance(scanner);
		return true;
	}
	return false;
}

static bool isAtEnd(Scanner* scanner) {
	return *(scanner->current) == '\0';
}



static char peek_next(Scanner* scanner) {
	return *((scanner->current + 1));
}

static Token create_token(Scanner *scanner, TokenType type) {
	Token tkn;
	tkn.length = (int) (scanner->current - scanner->start);
	tkn.start = scanner->start;
	tkn.type = type;
	tkn.line = scanner->line;
	return tkn;
}

static Token create_error_token(Scanner *scanner) {
	Token tkn;
	tkn.length = (int)(scanner->current - scanner->start);
	tkn.start = scanner->start;
	tkn.type = TOKEN_ERROR;
	tkn.line = scanner->line;
	return tkn;
}

Scanner create_token_scanner(const char* source) {
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
		return create_error_token(scanner);
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
	// run until you encounter a non numerical character.
	while (is_numeric(peek(scanner))) advance(scanner);

	// if encounter a .
	if (peek(scanner) == '.' && is_numeric(peek_next(scanner))) {
		// eat the .
		advance(scanner);
		// advance until encountering another non numerical character.
		// because we already ate the ., encoutering another . will not take it as a valid number. but a regular dot.
		while (is_numeric(peek(scanner))) advance(scanner);
	}

	return create_token(scanner, TOKEN_NUMBER);
}

static TokenType reserved_keywords(Scanner* scanner, int length, const char* rest, TokenType type) {
	// compare the length of the identifier, and compare the memory
	if (scanner->current - scanner->start == 1 + length && memcmp(scanner->start + 1, rest, length) == 0) {
		return type;
	}
	return TOKEN_IDENTIFIER;
}

static TokenType identifier_type(Scanner* scanner) {
	switch (*(scanner->start))
	{
		case 'v': return reserved_keywords(scanner, 2, "ar", TOKEN_VAR);
		case 'f': {
			if (scanner->current - scanner->start >= 1) {
				switch (scanner->start[1]) {
				case 'a': return reserved_keywords(scanner, 3, "lse", TOKEN_FALSE);
				case 'n': return TOKEN_FN;
				default: return TOKEN_IDENTIFIER;
				}
			}
		}
        case 'r': return reserved_keywords(scanner, 5, "eturn", TOKEN_RETURN);
		case 't': return reserved_keywords(scanner, 3, "rue", TOKEN_TRUE);
		case 'n': return reserved_keywords(scanner, 2, "il", TOKEN_NIL);
		case 'i': return reserved_keywords(scanner, 1, "f", TOKEN_IF);
		case 'p': return reserved_keywords(scanner, 4, "rint", TOKEN_PRINT);
		default:
			return TOKEN_IDENTIFIER;
	}
}

static Token identifier(Scanner* scanner) {
	while (is_alpha(peek(scanner)) || is_numeric(peek(scanner))) advance(scanner);

	return create_token(scanner, identifier_type(scanner));
}


static void remove_whitespaces(Scanner *scanner) {
	for (;;) {
		char c = peek(scanner);
		switch (c) {
			case ' ':
			case '\t':
			case '\n':
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
	// switch the char, and find the write token to create
	switch (c) {
		case '+': return create_token(scanner, TOKEN_PLUS);
		case '*': return create_token(scanner, TOKEN_STAR); 
		case '/': return create_token(scanner, TOKEN_SLASH); 
		case '-': return create_token(scanner, TOKEN_MINUS); 
		case '{': return create_token(scanner, TOKEN_LEFT_BRACE);
		case '}': return create_token(scanner, TOKEN_RIGHT_BRACE); 
		case '(': return create_token(scanner, TOKEN_LEFT_PAREN); 
		case ')': return create_token(scanner, TOKEN_RIGHT_PAREN); 
		case '.': return create_token(scanner, TOKEN_DOT);
		case ',': return create_token(scanner, TOKEN_COMMA);
		case '=': {
			return create_token(scanner, match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
		}
		case '!': {
			return create_token(scanner, match(scanner, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
		}
		case '>': {
			return create_token(scanner, match(scanner, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
		}
		case '<': {
			return create_token(scanner, match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
		}
		case ';': return create_token(scanner, TOKEN_SEMICOLON);
		case '"': return string(scanner);
		default: {
			if (is_numeric(c)) {
				return number(scanner);
			}
			if (is_alpha(c)) {
				return identifier(scanner);
			}
			return create_error_token(scanner);
		}
	}
}

Token tokenize(Scanner *scanner) {
	remove_whitespaces(scanner);
	if (isAtEnd(scanner)) {
		return create_token(scanner, TOKEN_EOF);
	}
	Token token = scan_token(scanner);
	return token;
	
}


