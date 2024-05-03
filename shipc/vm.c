
#include <stdio.h>
#include "vm.h"



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

static void runtime_error(const char* message, ...) {
	printf("[ERROR] Encountered runtime error: %s", message);
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
#define READ_BYTE() *vm->ip++
#define READ_CONSTANT() chunk->constants.arr[READ_BYTE()]


	vm->chunk = chunk;
	vm->ip = chunk->codes;

	for (;;) {
		uint8_t opcode = READ_BYTE();
		switch (opcode) {
		case OP_HALT: print_value(pop(vm)); return;
			case OP_CONSTANT: {
				push(vm, READ_CONSTANT());
				break;
			}
			case OP_NOT: {
				Value value = pop(vm);
				if (!IS_BOOL(value)) {
					runtime_error("'not' operator cannot be called on non boolean object");
					return;
				}
				push(vm, VAR_BOOL(!AS_BOOL(value)));
				break;
			}
			case OP_NEGATE: {
				Value value = pop(vm);
				if (!IS_NUMBER(value)) {
					runtime_error("unary operator cannot be called on non number object");
					return;
				}
				push(vm, VAR_NUMBER(-AS_NUMBER(value)));
				break;
			}
			case OP_MUL: {
				Value a = pop(vm);
				Value b = pop(vm);
				if (!IS_NUMBER(a) || !IS_NUMBER(b)) {
					runtime_error("* operator accepts only numbers");
					return;
				}
				// simple multi by 2 optimiziation
				double mul = AS_NUMBER(a) * AS_NUMBER(b);
				push(vm, VAR_NUMBER(mul));
				break;
			}
			case OP_ADD: {
				Value a = pop(vm);
				Value b = pop(vm);
				if (!IS_NUMBER(a) || !IS_NUMBER(b)) {
					runtime_error("+ operator accepts only numbers");
					return;
				}
				double mul = AS_NUMBER(a) + AS_NUMBER(b);
				push(vm, VAR_NUMBER(mul));
				break;
			}
			case OP_DIV: {
				Value b = pop(vm);
				Value a = pop(vm);
				if (!IS_NUMBER(a) || !IS_NUMBER(b)) {
					runtime_error("/ operator accepts only numbers");
					return;
				}
				double mul = AS_NUMBER(a) / AS_NUMBER(b);
				push(vm, VAR_NUMBER(mul));
				break;
			}
			case OP_SUB: {
				Value b = pop(vm);
				Value a = pop(vm);
				if (!IS_NUMBER(a) || !IS_NUMBER(b)) {
					runtime_error("/ operator accepts only numbers");
					return;
				}
				double mul = AS_NUMBER(a) - AS_NUMBER(b);
				push(vm, VAR_NUMBER(mul));
				break;
			}
			case OP_FALSE: {
				push(vm, VAR_BOOL(false));
				break;
			}
			case OP_TRUE: {
				push(vm, VAR_BOOL(true));
				break;
			}
			case OP_NIL: {
				push(vm, VAR_NIL);
				break;
			}
			case OP_POP_TOP: {
				pop(vm);
				break;
			}
			case OP_COMPARE: {
				Value b = pop(vm);
				Value a = pop(vm);
				if (a.type != b.type) {
					runtime_error("Cannot compare two different types");
					return;
				}
				bool equal = false;
				switch (a.type) {
				case VAL_BOOL: equal = AS_BOOL(a) == AS_BOOL(b); break;
				case VAL_NIL: equal = true; break;
				case VAL_NUMBER: equal = AS_NUMBER(a) == AS_NUMBER(b); break;
				}
				push(vm, VAR_BOOL(equal));
				break;
			}
			case OP_POP_JUMP_IF_FALSE: {
				Value cond = pop(vm);
				// if condition is truthy, then don't jump
				if (is_truthy(cond)) {
					READ_BYTE(); // the byte after the op is the jmp size, so avoid reading it
					break;
				}
				// if condition is false, jump
				Value jmp_size = READ_CONSTANT();
				vm->ip += (int) AS_NUMBER(jmp_size);
				break;

			}
		}
	}
#undef READ_BYTE
#undef READ_CONSTANT
}

