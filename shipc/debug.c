#include <stdio.h>

#include "debug.h"


static void simple_instruction(const char* string) {
	printf("|  %s  |\n", string);
}

static void constant_instruction(uint8_t value) {
	printf("|  OP_CONSTANT %u  |\n", value);
}

static void disassemble_instruction(Chunk* chunk, int offset) {
	uint8_t code = chunk->codes[offset];
	switch (code) {
		case OP_HALT: simple_instruction("OP_HALT");  break;
		case OP_NEGATE: simple_instruction("OP_NEGATE"); break;
		case OP_CONSTANT: constant_instruction(chunk->codes[offset + 1]); break;
		case OP_ADD: simple_instruction("OP_ADD"); break;
		case OP_SUB: simple_instruction("OP_SUB"); break;
		case OP_DIV: simple_instruction("OP_DIV"); break;
		case OP_MUL: simple_instruction("OP_MUL"); break;
	}
}

void disassemble_chunk(Chunk* chunk) {
	printf("=== disassembled chunk ===\n");
	for (int i = 0; i < chunk->count; i++) {
		disassemble_instruction(chunk, i);
	}
}