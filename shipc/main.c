#include <stdio.h>


#include "token.h"
#include "chunk.h"
#include "vm.h"

int main() {
	Chunk chn;
	init_chunk(&chn);
	add_code(&chn, OP_HALT);
	return 0;
}