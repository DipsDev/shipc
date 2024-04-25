#include "vm.h"


#define READ_BYTE(vm) *vm->ip++


static void push(VM* vm, Value value) {
	if ((size_t)(vm->sp - vm->stack) == STACK_MAX) {
		printf("Stack overflow");
		exit(1);
	}
	*vm->sp = value;
	vm->sp++;
}

static Value pop(VM* vm) {
	if (vm->stack == vm->sp) {
		printf("Empty stack");
		exit(1);
	}
	vm->sp--;
	return *vm->sp;
}

void init_vm(VM* vm) {
	// set the ip to null
	vm->ip = NULL;

	// set the sp to the beginning of the stack
	vm->sp = vm->stack;
}

void free_vm(VM* vm) {

}

void interpret(VM* vm, Chunk* chunk) {
	vm->chunk = chunk;
	vm->ip = chunk->codes;

	for (;;) {
		uint8_t opcode = READ_BYTE(vm);
		switch (opcode) {
			case OP_HALT: return;
			case OP_CONSTANT: {
				uint8_t constant = READ_BYTE(vm);
				push(vm, NUMBER(constant));
			}
			case OP_NEGATE: {
				Value value = pop(vm);
				push(vm, NUMBER(-value));
			}
		}
	}
}