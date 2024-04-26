#include <stdio.h>
#include "compiler.h"


int main() {
	Chunk chunk;
	init_chunk(&chunk);

	compile("Hello World!", &chunk);



	return 0;
}