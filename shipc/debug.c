#include <stdio.h>

#include "debug.h"


static int simple_instruction(const char* string) {
	printf("|  %s  |\n", string);
	return 1;
}

static int constant_instruction(Chunk* chunk, int offset) {
	uint8_t index = chunk->codes[offset + 1];
	printf("|  OP_CONSTANT %u (%.2f) |\n", index, chunk->constants.arr[index]);
	return 2;
}

static int disassemble_instruction(Chunk* chunk, int offset) {
	uint8_t code = chunk->codes[offset];
	switch (code) {
		case OP_HALT: return simple_instruction("OP_HALT");
		case OP_NEGATE: return simple_instruction("OP_NEGATE");
		case OP_ADD: return simple_instruction("OP_ADD");
		case OP_SUB: return simple_instruction("OP_SUB");
		case OP_DIV: return simple_instruction("OP_DIV");
		case OP_MUL: return simple_instruction("OP_MUL");
		case OP_CONSTANT: return constant_instruction(chunk, offset);
	}
}

void disassemble_chunk(Chunk* chunk) {
	printf("=== disassembled chunk l(%i) ===\n", chunk->count);
	for (int i = 0; i < chunk->count;) {
		i += disassemble_instruction(chunk, i);
	}
}