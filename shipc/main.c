#include <stdio.h>
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main() {
	Chunk chunk;
	init_chunk(&chunk);

	int constant = add_constant(&chunk, NUMBER(40));
	write_chunk(&chunk, OP_CONSTANT);
	write_chunk(&chunk, constant);
	write_chunk(&chunk, OP_HALT);
	disassemble_chunk(&chunk);

	VM vm;
	init_vm(&vm);
	interpret(&vm, &chunk);
	
	free_vm(&vm);
	free_chunk(&chunk);
	return 0;
}