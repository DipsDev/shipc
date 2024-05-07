
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "vm.h"
#include "objects.h"



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
	va_list args;
	va_start(args, message);
	printf("[ERROR] Encountered runtime error: ");
	vprintf(message, args);
	va_end(args);
}

void init_vm(VM* vm) {
	// set the ip to null
	vm->ip = NULL;

	// set the sp to the beginning of the stack
	vm->sp = vm->stack;

	vm->globals = (HashMap*) malloc(sizeof(HashMap));
	create_variable_map(vm->globals);
}

void free_vm(VM* vm) {
	free_hash_map(vm->globals);
}

void interpret(VM* vm, Chunk* chunk) {
#define READ_BYTE() *vm->ip++
#define READ_CONSTANT() chunk->constants.arr[READ_BYTE()]
#define READ_SHORT() \
	(vm->ip += 2, (uint16_t) ((vm->ip[-2] << 8) | vm->ip[-1]))


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
				Value b = pop(vm);
				Value a = pop(vm);
				if (IS_NUMBER(a) && IS_NUMBER(b)) {
					double mul = AS_NUMBER(a) + AS_NUMBER(b);
					push(vm, VAR_NUMBER(mul));
					break;
				}
				if (IS_STRING(a) && IS_STRING(b)) {
					// should return a + b;
					StringObj* str_a = AS_STRING(a);
					StringObj* str_b = AS_STRING(b);

					
					StringObj* concat = concat_strings(str_a->value, str_a->length, str_b->value, str_b->length);

					push(vm, VAR_OBJ(concat));
					break;
				}
				runtime_error("unknown operands for '+' operator");
				return;
				
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
					push(vm, VAR_BOOL(false));
					break;
				}
				bool equal = false;
				switch (a.type) {
				case VAL_BOOL: equal = AS_BOOL(a) == AS_BOOL(b); break;
				case VAL_NIL: equal = true; break;
				case VAL_NUMBER: equal = AS_NUMBER(a) == AS_NUMBER(b); break;
				case VAL_OBJ: equal = compare_objects(AS_OBJ(a), AS_OBJ(b));
				}
				push(vm, VAR_BOOL(equal));
				break;
			}
			case OP_POP_JUMP_IF_FALSE: {
				Value cond = pop(vm);
				// if condition is truthy, then don't jump
				if (is_truthy(cond)) {
					(void) READ_SHORT(); // the byte after the op is the jmp size, so avoid reading it
					break;
				}
				// if condition is false, jump
				uint16_t jmp_size = READ_SHORT();
				vm->ip += (int) jmp_size;
				break;

			}
			case OP_STORE_GLOBAL: {
				Value var_value = pop(vm);
				Value variable_name = READ_CONSTANT();

				if (!IS_STRING(variable_name)) {
					runtime_error("expected variable name to be string");
					exit(1);
				}
				StringObj* obj = AS_STRING(variable_name);
				put_node(vm->globals, obj->value, obj->length, var_value);
				break;
			}
			case OP_ASSIGN_GLOBAL: {
				Value var_value = pop(vm);
				Value variable_name = READ_CONSTANT();

				if (!IS_STRING(variable_name)) {
					runtime_error("expected variable name to be string");
					exit(1);
				}
				
				StringObj* obj = AS_STRING(variable_name);
				HashNode* var_node = get_node(vm->globals, obj->value, obj->length);
				if (var_node == NULL) {
					runtime_error("variable name '%.*s' is undefined", obj->length, obj->value);
					exit(1);
				}
				var_node->value = var_value;
				break;
			}
			case OP_LOAD_GLOBAL: {
				Value variable_name = READ_CONSTANT();
				if (!IS_STRING(variable_name)) {
					runtime_error("expected variable name to be string");
					exit(1);
				}
				StringObj* obj = AS_STRING(variable_name);
				HashNode* var_node = get_node(vm->globals, obj->value, obj->length);
				if (var_node == NULL) {
					runtime_error("variable name '%.*s' is undefined", obj->length, obj->value);
					exit(1);
				}
				push(vm, var_node->value);
				break;
			}
			case OP_CALL: {
				Value func_name = READ_CONSTANT();
				if (!IS_STRING(func_name)) {
					runtime_error("expected function name to be string");
					exit(1);
				}
				StringObj* obj = AS_STRING(func_name);
				HashNode* var_node = get_node(vm->globals, obj->value, obj->length);
				if (var_node == NULL) {
					runtime_error("function name '%.*s' is undefined", obj->length, obj->value);
					exit(1);
				}
				break;
			}
		}
	}
#undef READ_SHORT
#undef READ_BYTE
#undef READ_CONSTANT
}

