#include <stdio.h>
#include "debug.h"
#include "compiler.h"
#include "vm.h"


int main() {
	Chunk chunk;
	init_chunk(&chunk);

	if (!compile("4 + 4 * (3 + 2)", &chunk)) {
		free_chunk(&chunk);
		exit(1);
	}
	disassemble_chunk(&chunk);

	VM vm;
	init_vm(&vm);
	interpret(&vm, &chunk);

	free_vm(&vm);
	free_chunk(&chunk);



	return 0;
}