#include <stdio.h>

#include "debug.h"


static int simple_instruction(const char* string, int offset) {
	printf("| %04d  %s  |\n", offset, string);
	return 1;
}

static int constant_instruction(Chunk* chunk, int offset) {
	uint8_t index = chunk->codes[offset + 1];
	Value val = chunk->constants.arr[index];
	switch (val.type) {
	case VAL_BOOL: printf("| %04d OP_CONSTANT %u (%s) |\n", offset, index, AS_BOOL(val) ? "true" : "false"); break;
	case VAL_NIL: printf("| %04d OP_CONSTANT %u (nil) |\n", offset, index); break;
	case VAL_NUMBER: printf("| %04d OP_CONSTANT %u (%.2f) |\n", offset, index, AS_NUMBER(val)); break;
	}
	return 2;
}

static int disassemble_instruction(Chunk* chunk, int offset) {
	uint8_t code = chunk->codes[offset];
	switch (code) {
		case OP_HALT: return simple_instruction("OP_HALT", offset);
		case OP_NEGATE: return simple_instruction("OP_NEGATE", offset);
		case OP_ADD: return simple_instruction("OP_ADD", offset);
		case OP_SUB: return simple_instruction("OP_SUB", offset);
		case OP_DIV: return simple_instruction("OP_DIV", offset);
		case OP_MUL: return simple_instruction("OP_MUL", offset);
		case OP_FALSE: return simple_instruction("OP_FALSE", offset);
		case OP_NOT: return simple_instruction("OP_NOT", offset);
		case OP_TRUE: return simple_instruction("OP_TRUE", offset);
		case OP_NIL:return simple_instruction("OP_NIL", offset);
		case OP_CONSTANT: return constant_instruction(chunk, offset);


	}
}

void disassemble_chunk(Chunk* chunk) {
	printf("=== disassembled chunk l(%i) ===\n", chunk->count);
	for (int i = 0; i < chunk->count;) {
		i += disassemble_instruction(chunk, i);
	}
}