#include <stdio.h>

#include "debug.h"


static void simple_instruction(const char* string) {
	printf("|  %s  |", string);
}

static void disassemble_instruction(uint8_t code) {
	switch (code) {
		case OP_HALT: simple_instruction("OP_HALT");  break;
	}
}

void disassemble_chunk(Chunk* chunk) {
	printf("=== disassembled chunk ===\n");
	for (int i = 0; i < chunk->count; i++) {
		disassemble_instruction(chunk->codes[i]);
	}
}