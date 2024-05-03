#include <stdio.h>
#include "debug.h"
#include "compiler.h"
#include "vm.h"
#include <stdlib.h>


int main() {
	Chunk chunk;
	init_chunk(&chunk);

	const char* source_code =
		"if 4 == 6 { 4 + 5 / 2 == 4;} 4 + 3 * 3 == 6;";

	if (!compile(source_code, &chunk)) {
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
