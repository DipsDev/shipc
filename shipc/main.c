#include "token.h"

int main() {
	char* source = "var a = 4.45; 21 <= 4";
	tokenize(source);
	return 0;
}