

#include "vm.h"

static uint8_t advance(VM* vm) {
	vm->ip++;
	return vm->ip[-1];
}

void init_vm(VM* vm) {
	vm->chunk = NULL;
	vm->ip = NULL;
}

void set_vm_chunk(VM* vm, Chunk* chunk) {
	vm->chunk = chunk;
	vm->ip = chunk->codes;
}

void interpret(VM* vm) {
	uint8_t code = advance(vm);
	for (;;) {
		switch (code) {
			case OP_HALT: return;
		}
	}
}

