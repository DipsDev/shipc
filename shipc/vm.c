
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "vm.h"
#include "objects.h"

static InterpretResult run (VM* vm);

static void push(VM* vm, Value value) {
	if ((size_t)(vm->sp - vm->stack) == STACK_MAX) {
		printf("Stack overflow");
		exit(1);
	}
	*vm->sp = value;
	vm->sp++;
}

static void free_stack_frame(StackFrame* frame) {
    free_object(&frame->function->obj);
}

static void push_frame(VM* vm, StackFrame frame) {
    if (vm->frameCount >= CALL_STACK_MAX) {
        printf("max frames reached");
        exit(1);
    }
    vm->callStack[vm->frameCount] = frame;
    vm->frameCount++;
}

static Value pop(VM* vm) {
	if (vm->stack == vm->sp) {
		printf("Empty stack");
		exit(1);
	}
	vm->sp--;
	return *vm->sp;
}

static InterpretResult runtime_error(VM* vm, const char* message, ErrorType type, ...) {
    // creates an informative error message, and returns it
    char temp[256] = {0};
    va_list args;
    va_start(args, type);
    vsnprintf(temp, 255, message, args);
    va_end(args);
    ErrorObj* err = create_err_obj(temp, (int) strlen(temp), type);
    push(vm, VAR_OBJ(err));
    return RESULT_ERROR;
}


void init_vm(VM* vm) {
	// set the frame count to 0
    vm->frameCount = 0;
	// set the sp to the beginning of the stack
	vm->sp = vm->stack;

}

void free_vm(VM* vm) {
}

void interpret(VM* vm, FunctionObj* main_script) {
    // push the main script to the call stack
    StackFrame main_frame;
    main_frame.ip = main_script->body.codes;
    main_frame.function = main_script;
    push_frame(vm, main_frame);

    InterpretResult end_value = run(vm);
    if(end_value == RESULT_ERROR) {
        Value error_value = pop(vm);
        ErrorObj* err_obj = (ErrorObj*) AS_OBJ(error_value);
        fprintf(stderr, "[ERROR] %.*s", err_obj->value->length, err_obj->value->value);
        return;
    }
}

static InterpretResult run(VM* vm) {
    StackFrame* frame = &vm->callStack[vm->frameCount - 1];
#define READ_BYTE() (*frame->ip++)
#define READ_CONSTANT() frame->function->body.constants.arr[READ_BYTE()]
#define READ_SHORT() \
	(frame->ip += 2, (uint16_t) ((frame->ip[-2] << 8) | frame->ip[-1]))

	for (;;) {
		uint8_t opcode = READ_BYTE();
		switch (opcode) {
            case OP_RETURN: {
                if (frame->function->type == FN_SCRIPT) {
                    return runtime_error(vm, "'return' outside of function", ERR_SYNTAX);
                }
                // clear the current frame
                // free_stack_frame(frame);
                vm->frameCount--;
                // set the new frame
                frame = &vm->callStack[vm->frameCount - 1];
                break;

            }
            case OP_HALT: {
                // free_stack_frame(frame);
                return RESULT_SUCCESS;
            }
			case OP_CONSTANT: {
				push(vm, READ_CONSTANT());
				break;
			}
			case OP_NOT: {
				Value value = pop(vm);
				if (!IS_BOOL(value)) {
					return runtime_error(vm, "'not' operator cannot be called on non boolean object", ERR_TYPE);
				}
				push(vm, VAR_BOOL(!AS_BOOL(value)));
				break;
			}
			case OP_NEGATE: {
				Value value = pop(vm);
				if (!IS_NUMBER(value)) {
                    return runtime_error(vm, "unary operator cannot be called on non number object", ERR_TYPE);
				}
				push(vm, VAR_NUMBER(-AS_NUMBER(value)));
				break;
			}
			case OP_MUL: {
				Value a = pop(vm);
				Value b = pop(vm);
				if (!IS_NUMBER(a) || !IS_NUMBER(b)) {
					return runtime_error(vm, "* operator accepts only numbers", ERR_TYPE);

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
                // if one of the values is a string, then cast everything to a string and concat it.
				if (IS_STRING(a) || IS_STRING(b)) {
                    // TODO: Implement string concatenation with other types
					StringObj* str_a = AS_STRING(a);
					StringObj* str_b = AS_STRING(b);

					
					StringObj* concat = concat_strings(str_a->value, str_a->length, str_b->value, str_b->length);

					push(vm, VAR_OBJ(concat));
					break;
				}
				return runtime_error(vm, "unknown operands for '+' operator", ERR_TYPE);

			}
			case OP_DIV: {
				Value b = pop(vm);
				Value a = pop(vm);
				if (!IS_NUMBER(a) || !IS_NUMBER(b)) {
					return runtime_error(vm, "/ operator accepts only numbers", ERR_TYPE);
				}
				double mul = AS_NUMBER(a) / AS_NUMBER(b);
				push(vm, VAR_NUMBER(mul));
				break;
			}
			case OP_SUB: {
				Value b = pop(vm);
				Value a = pop(vm);
				if (!IS_NUMBER(a) || !IS_NUMBER(b)) {
					return runtime_error(vm, "/ operator accepts only numbers", ERR_TYPE);
				}
				double mul = AS_NUMBER(a) - AS_NUMBER(b);
				push(vm, VAR_NUMBER(mul));
				break;
			}
            case OP_SHOW_TOP: {
                print_value(pop(vm));
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
				frame->ip += (int) jmp_size;
				break;

			}
			case OP_STORE_GLOBAL: {
				Value var_value = pop(vm);
				Value variable_name = READ_CONSTANT();

				if (!IS_STRING(variable_name)) {
					return runtime_error(vm, "expected variable name to be string", ERR_TYPE);
				}
				StringObj* obj = AS_STRING(variable_name);
                if (get_node(vm->globals, obj->value, obj->length) != NULL) {
                    return runtime_error(vm, "variable '%.*s' is already defined in the current scope", ERR_NAME, obj->length, obj->value);
                }
				put_node(vm->globals, obj->value, obj->length, var_value);
				break;
			}
			case OP_ASSIGN_GLOBAL: {
                return runtime_error(vm, "variable assignment is yet to be implemented", ERR_SYNTAX);
			}
			case OP_LOAD_GLOBAL: {
				Value variable_name = READ_CONSTANT();
				if (!IS_STRING(variable_name)) {
					return runtime_error(vm, "expected variable name to be string", ERR_TYPE);
				}
				StringObj* obj = AS_STRING(variable_name);
				HashNode* var_node = get_node(vm->globals, obj->value, obj->length);
				if (var_node == NULL) {
					return runtime_error(vm, "variable '%.*s' is undefined", ERR_NAME, obj->length, obj->value);
				}
				push(vm, var_node->value);
				break;
			}
			case OP_CALL: {
				Value func_name = pop(vm);
				if (!IS_FUNCTION(func_name)) {
					return runtime_error(vm, "object is not callable", ERR_TYPE);
				}
				FunctionObj * obj = AS_FUNCTION(func_name);
				HashNode* var_node = get_node(vm->globals, obj->name->value, obj->name->length);
				if (var_node == NULL) {
					return runtime_error(vm, "function '%.*s' is undefined", ERR_NAME, obj->name->length, obj->name->value);
				}
                StackFrame new_func;
                new_func.function = obj;
                new_func.ip = new_func.function->body.codes;
                push_frame(vm, new_func);
                frame = &new_func;
                break;

			}
            default:
                return runtime_error(vm, "unhandled op code %d", ERR_SYNTAX, opcode);
		}
	}
#undef READ_SHORT
#undef READ_BYTE
#undef READ_CONSTANT
}

