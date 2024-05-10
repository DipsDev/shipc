
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "vm.h"
#include "objects.h"

static Value run (VM* vm, FunctionObj* script, unsigned int scope);

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

static Value runtime_error(const char* message, ErrorType type, ...) {
    // creates an informative error message, and returns it
    char temp[256] = {0};
    va_list args;
    va_start(args, type);
    vsnprintf(temp, 255, message, args);
    va_end(args);
    ErrorObj* err = create_err_obj(temp, (int) strlen(temp), type);
    return VAR_OBJ(err);
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

void interpret(VM* vm, FunctionObj* main_script) {
    Value end_value = run(vm, main_script, 0);
    if(IS_ERROR(end_value)) {
        ErrorObj* err_obj = (ErrorObj*) AS_OBJ(end_value);
        fprintf(stderr, "[ERROR] %.*s", err_obj->value->length, err_obj->value->value);
        return;
    }
    print_value(pop(vm));
}

static Value run(VM* vm, FunctionObj* script, unsigned int scope) {
#define READ_BYTE() *vm->ip++
#define READ_CONSTANT() script->body.constants.arr[READ_BYTE()]
#define READ_SHORT() \
	(vm->ip += 2, (uint16_t) ((vm->ip[-2] << 8) | vm->ip[-1]))


	vm->chunk = &script->body;
	vm->ip = vm->chunk->codes;

	for (;;) {
		uint8_t opcode = READ_BYTE();
		switch (opcode) {
            case OP_RETURN: {
                if (scope == 0) {
                    return runtime_error("'return' outside of function", ERR_SYNTAX);
                }
                return pop(vm);
            }
            case OP_HALT: return VAR_NIL;
			case OP_CONSTANT: {
				push(vm, READ_CONSTANT());
				break;
			}
			case OP_NOT: {
				Value value = pop(vm);
				if (!IS_BOOL(value)) {
					return runtime_error("'not' operator cannot be called on non boolean object", ERR_TYPE);
				}
				push(vm, VAR_BOOL(!AS_BOOL(value)));
				break;
			}
			case OP_NEGATE: {
				Value value = pop(vm);
				if (!IS_NUMBER(value)) {
                    return runtime_error("unary operator cannot be called on non number object", ERR_TYPE);
				}
				push(vm, VAR_NUMBER(-AS_NUMBER(value)));
				break;
			}
			case OP_MUL: {
				Value a = pop(vm);
				Value b = pop(vm);
				if (!IS_NUMBER(a) || !IS_NUMBER(b)) {
					return runtime_error("* operator accepts only numbers", ERR_TYPE);

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
				return runtime_error("unknown operands for '+' operator", ERR_TYPE);

			}
			case OP_DIV: {
				Value b = pop(vm);
				Value a = pop(vm);
				if (!IS_NUMBER(a) || !IS_NUMBER(b)) {
					return runtime_error("/ operator accepts only numbers", ERR_TYPE);
				}
				double mul = AS_NUMBER(a) / AS_NUMBER(b);
				push(vm, VAR_NUMBER(mul));
				break;
			}
			case OP_SUB: {
				Value b = pop(vm);
				Value a = pop(vm);
				if (!IS_NUMBER(a) || !IS_NUMBER(b)) {
					return runtime_error("/ operator accepts only numbers", ERR_TYPE);
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
					return runtime_error("expected variable name to be string", ERR_TYPE);
				}
				StringObj* obj = AS_STRING(variable_name);
				put_node(vm->globals, obj->value, obj->length, var_value);
				break;
			}
			case OP_ASSIGN_GLOBAL: {
				Value var_value = pop(vm);
				Value variable_name = READ_CONSTANT();

				if (!IS_STRING(variable_name)) {
					return runtime_error("expected variable name to be string", ERR_TYPE);
				}
				
				StringObj* obj = AS_STRING(variable_name);
				HashNode* var_node = get_node(vm->globals, obj->value, obj->length);
				if (var_node == NULL) {
					return runtime_error("variable name '%.*s' is undefined", ERR_NAME, obj->length, obj->value);
				}
				var_node->value = var_value;
				break;
			}
			case OP_LOAD_GLOBAL: {
				Value variable_name = READ_CONSTANT();
				if (!IS_STRING(variable_name)) {
					return runtime_error("expected variable name to be string", ERR_TYPE);
				}
				StringObj* obj = AS_STRING(variable_name);
				HashNode* var_node = get_node(vm->globals, obj->value, obj->length);
				if (var_node == NULL) {
					return runtime_error("variable '%.*s' is undefined", ERR_NAME, obj->length, obj->value);
				}
				push(vm, var_node->value);
				break;
			}
			case OP_CALL: {
				Value func_name = pop(vm);
				if (!IS_FUNCTION(func_name)) {
					return runtime_error("expected function to be a function", ERR_TYPE);
				}
				FunctionObj * obj = AS_FUNCTION(func_name);
				HashNode* var_node = get_node(vm->globals, obj->name->value, obj->name->length);
				if (var_node == NULL) {
					return runtime_error("function '%.*s' is undefined", ERR_NAME, obj->name->length, obj->name->value);
				}
				if (!IS_FUNCTION(var_node->value)) {
					return runtime_error("object '%.*s' is not callable.", ERR_NAME, obj->name->length, obj->name->value);
				}
                uint8_t * jump_address = vm->ip;
                // TODO: The run should NOT be recursive, but push the function to the call stack and make the run function execute the function at the top of the stack.
                Value return_value = run(vm, obj, scope + 1);
                // if function was error, then error it up - currently
                if (IS_ERROR(return_value)) {
                    return return_value;
                }

                push(vm, return_value);

                // return the jump address to the right place
                vm->ip = jump_address;
                vm->chunk = &script->body;
                break;

			}
            default:
                return runtime_error("unhandled op code %d", ERR_SYNTAX, opcode);
		}
	}
#undef READ_SHORT
#undef READ_BYTE
#undef READ_CONSTANT
}

