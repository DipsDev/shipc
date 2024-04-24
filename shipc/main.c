#include <stdio.h>

#include "token.h"

int main() {
	char* source = "var a = 4.45; 21 <= 4";
	Scanner scanner = create_token_scanner(source);
	for (;;) {
		Token token = tokenize(&scanner);
		
		if (token.type == TOKEN_EOF) break;

		printf("%2d '%.*s'\n", token.type, token.length, token.start);
	}
	return 0;
}