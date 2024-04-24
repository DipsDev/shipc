#include "token.h"

int main() {
	char* source = "\" Hello World\" + 5.345 / my_variable";
	tokenize(source);
	return 0;
}