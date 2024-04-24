#include <stdio.h>


#include "token.h"
#include "chunk.h"
#include "vm.h"

int main() {
	Chunk chn;
	init_chunk(&chn);
	add_code(&chn, OP_HALT);

	print_chunk(&chn);

	VM vm;
	init_vm(&vm);
	set_vm_chunk(&vm, &chn);

	interpret(&vm);

	return 0;
}