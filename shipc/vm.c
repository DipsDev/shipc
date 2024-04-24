#include "vm.h"

static uint8_t advance(VM* vm) {
	return *vm->ip++;
}

static Value pop(VM* vm) {
	return *vm->stack_top--;
}

void free_vm(VM* vm) {
}

void init_vm(VM* vm) {
	vm->chunk = NULL;
	vm->ip = NULL;
	vm->stack_top = vm->stack;
}

void set_vm_chunk(VM* vm, Chunk* chunk) {
	vm->chunk = chunk;
	vm->ip = chunk->codes;
}


void handle_constant(VM* vm) {
	uint8_t constant = advance(vm);
}

void interpret(VM* vm) {
	uint8_t code = advance(vm);
	for (;;) {
		switch (code) {
			case OP_HALT: return;
		}
	}
}

