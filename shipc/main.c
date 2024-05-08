#include <stdio.h>
#include "debug.h"
#include "compiler.h"
#include "vm.h"
#include <stdlib.h>


int main() {
	const char* source_code =
		"var y = 14; var x = 13; print(x * y + y - x);";

	FunctionObj* compiled_func = compile(source_code);
	if (compiled_func == NULL) {
		exit(1);
	}
	disassemble_func(compiled_func);

	VM vm;
	init_vm(&vm);
	interpret(&vm, compiled_func);

	free_vm(&vm);



	return 0;
}
