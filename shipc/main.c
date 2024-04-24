#include <stdio.h>
#include "chunk.h"
#include "debug.h"

int main() {
	Chunk chunk;
	init_chunk(&chunk);
	write_chunk(&chunk, OP_HALT);
	disassemble_chunk(&chunk);
	free_chunk(&chunk);
	return 0;
}