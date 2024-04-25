#include <stdio.h>
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main() {
	Chunk chunk;
	init_chunk(&chunk);
	write_chunk(&chunk, OP_CONSTANT);
	write_chunk(&chunk, 4);
	write_chunk(&chunk, OP_NEGATE);
	write_chunk(&chunk, OP_HALT);
	disassemble_chunk(&chunk);

	VM vm;
	init_vm(&vm);
	interpret(&vm, &chunk);
	
	free_vm(&vm);
	free_chunk(&chunk);
	return 0;
}