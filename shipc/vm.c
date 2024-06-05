
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "vm.h"
#include "memory.h"
#include "objects.h"
#include "builtins.h"

static InterpretResult run (VM* vm);

static void push(VM* vm, Value value) {
	if ((size_t)(vm->sp - vm->stack) == STACK_MAX) {
		printf("Stack overflow");
		exit(1);
	}
	*vm->sp = value;
	vm->sp++;
}

static void free_stack_frame(StackFrame frame) {
    free_object((Obj *) frame.function);
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

static Value peek_behind(VM* vm, int behind) {
    if (vm->stack >  vm->sp - behind) {
        printf("Empty stack");
        exit(1);
    }
    return *(vm->sp - behind);
}

static void throw_error(VM* vm, ErrorObj* err) {
    StackFrame errored_chunk = vm->callStack[vm->frameCount - 1];
    int code_offset = (errored_chunk.ip - errored_chunk.function->body.codes);

    fprintf(stderr, "runtime error: %.*s\n  [main.ship:%i]\n",
            err->value->length, err->value->value, errored_chunk.function->body.lines[code_offset]);
    exit(1);
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




Value native_time(int arg_count, Value* args) {
    return VAR_NUMBER((double) clock() / CLOCKS_PER_SEC);
}

void init_vm(VM* vm) {
	// set the frame count to 0
    vm->frameCount = 0;
	// set the sp to the beginning of the stack
	vm->sp = vm->stack;

    // create the objects arrays
    vm->objects = NULL;
    vm->heapObjects = 0;
    vm->heapCapacity = 8;

    ValueTable globals;
    create_value_map(&globals);
    vm->globals = globals;

    NativeFuncObj * fn = create_native_func_obj(native_time);
    put_value_node(&vm->globals, "time", 4, VAR_OBJ(fn));

}

void free_vm(VM* vm) {
    // Free all the gc objects
    Obj* pos = (Obj *) vm->objects;
    while (pos != NULL) {
        Obj* next = (Obj *) pos->next;
        free_object(pos);
        pos = next;
    }

    // Free the last stackframe
    free_stack_frame(vm->callStack[0]);

    // Free the globals
    free_globals(&vm->globals);
}

InterpretResult interpret(VM* vm, FunctionObj* main_script) {
    // push the main script to the call stack
    StackFrame main_frame;
    main_frame.ip = main_script->body.codes;
    main_frame.function = main_script;
    push_frame(vm, main_frame);

    InterpretResult end_value = run(vm);
    if(end_value == RESULT_ERROR) {
        Value error_value = pop(vm);
        ErrorObj* err_obj = (ErrorObj*) AS_OBJ(error_value);
        throw_error(vm, err_obj);
        return RESULT_ERROR;
    }
    return RESULT_SUCCESS;
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
                Value constant = READ_CONSTANT();
				push(vm, constant);
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
            case OP_MODULO: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (!IS_NUMBER(a) || !IS_NUMBER(b)) {
                    return runtime_error(vm, "modulos operator accepts only number types", ERR_TYPE);
                }
                double mod = fmod(AS_NUMBER(a), AS_NUMBER(b));
                push(vm, VAR_NUMBER(mod));
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
            case OP_LESS_THAN: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (IS_NUMBER(a) && IS_NUMBER(b)) {
                    bool test = AS_NUMBER(a) < AS_NUMBER(b);
                    push(vm, VAR_BOOL(test));
                    break;
                }
                return runtime_error(vm, "non supported operands for GREATER_THAN", ERR_TYPE);
            }
            case OP_GREATER_THAN: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (IS_NUMBER(a) && IS_NUMBER(b)) {
                    bool test = AS_NUMBER(a) > AS_NUMBER(b);
                    push(vm, VAR_BOOL(test));
                    break;
                }
                return runtime_error(vm, "non supported operands for GREATER_THAN", ERR_TYPE);
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
				if (IS_STRING(a) && IS_STRING(b)) {
					StringObj* str_a = AS_STRING(a);
					StringObj* str_b = AS_STRING(b);

					StringObj* concat = concat_strings(str_a->value, str_a->length, str_b->value, str_b->length);
                    add_garbage(vm, VAR_OBJ(concat));



					push(vm, VAR_OBJ(concat));
					break;
				}
				return runtime_error(vm, "unknown operands for '+' operator. have you considered using .to_str()?", ERR_TYPE);

			}
			case OP_DIV: {
				Value b = pop(vm);
				Value a = pop(vm);
				if (!IS_NUMBER(a) || !IS_NUMBER(b)) {
					return runtime_error(vm, "/ operator accepts only numbers", ERR_TYPE);
				}
                if (AS_NUMBER(b) == 0) {
                    return runtime_error(vm, "cannot divide by 0", ERR_SYNTAX);
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
                Value print_val = pop(vm);
                print_value(print_val);
                printf("\n");
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
            case OP_JUMP: {
                uint16_t jmp_size = READ_SHORT();
                frame->ip += (int) jmp_size;
                break;

            }
            case OP_JUMP_BACKWARD: {
                uint16_t jmp_size = READ_SHORT();
                frame->ip -= (int) jmp_size;
                break;
            }
			case OP_STORE_FAST: {
				Value var_value = pop(vm);
				uint8_t variable_index = READ_BYTE();
                frame->function->locals[variable_index].value = var_value;
				break;
			}
            case OP_LOAD_ATTR: {
                Value attr_name = READ_CONSTANT();
                Value attr_host = peek_behind(vm, 1);

                if (!IS_STRING(attr_name)) {
                    return runtime_error(vm, "Attribute name is expected to be a string", ERR_TYPE);
                }
                if (IS_CLASS(attr_host)) {
                    // Classes are not implemented in ship yet..
                    break;
                }
                Value attr_res = get_builtin_attr(attr_host, AS_STRING(attr_name));
                if (IS_ERROR(attr_res)) {
                    throw_error(vm, (ErrorObj*) AS_OBJ(attr_res));
                }
                add_garbage(vm, attr_res);
                push(vm, attr_res);
                break;
            }
            case OP_BUILD_ARRAY: {
                // Read the argument count
                uint8_t arg_count = READ_BYTE();

                ArrayObj* arr = create_array_obj();

                for(uint8_t i = arg_count; i > 0; i--) {
                    write_value_array(arr->values, vm->sp[-i]);
                }
                vm->sp -= arg_count;

                push(vm, VAR_OBJ(arr));
                // add_garbage(vm, VAR_OBJ(arr));
                break;
            }
			case OP_ASSIGN_GLOBAL: {
                Value  var_name = READ_CONSTANT();
                if (!IS_STRING(var_name)) {
                    return runtime_error(vm, "global variable should be a string.", ERR_SYNTAX);
                }
                StringObj* var_str = AS_STRING(var_name);
                // loop in other frames for the value until found. this is why local variables are faster than globals;
                for (int i = vm->frameCount - 2; i >= 0; i--) {
                    StackFrame* curr = &vm->callStack[i];
                    // iterate over the function locals and search for the value
                    for(int j = 0; j< curr->function->localCount; j++) {
                        Local current_local = curr->function->locals[j];
                        if (current_local.length == var_str->length && strncmp(current_local.name, var_str->value, current_local.length) == 0) {
                            curr->function->locals[j].value = pop(vm);
                            goto var_found;
                        }
                    }
                }
                return runtime_error(vm, "variable '%.*s' is not defined", ERR_NAME, var_str->length, var_str->value);
			}
            case OP_LOAD_LOCAL: {
                uint8_t variable_index = READ_BYTE();
                push(vm, frame->function->locals[variable_index].value);
                break;
            }
            case OP_ASSIGN_LOCAL: {
                Value val = pop(vm);
                uint8_t variable_index = READ_BYTE();
                frame->function->locals[variable_index].value = val;
                break;
            }
			case OP_LOAD_GLOBAL: {
                Value  var_name = READ_CONSTANT();
                if (!IS_STRING(var_name)) {
                    return runtime_error(vm, "global variable should be a string.", ERR_SYNTAX);
                }
                StringObj* var_str = AS_STRING(var_name);
                // loop in other frames for the value until found. this is why local variables are faster than globals;
                for (int i = vm->frameCount - 2; i >= 0; i--) {
                    StackFrame* curr = &vm->callStack[i];
                    // iterate over the function locals and search for the value
                    for(int j = 0; j< curr->function->localCount; j++) {
                        Local current_local = curr->function->locals[j];
                        if (current_local.length == var_str->length && strncmp(current_local.name, var_str->value, current_local.length) == 0) {
                            push(vm, current_local.value);
                            goto var_found;
                        }
                    }
                }
                ValueNode * glob = get_global(&vm->globals, var_str->value, var_str->length);
                if (glob != NULL) {
                    push(vm, glob->val);
                    goto var_found;
                }
                return runtime_error(vm, "variable '%.*s' is not defined", ERR_NAME, var_str->length, var_str->value);
                var_found:
                    break;
			}
            case OP_GET_ITER: {
                Value to_get_iter = pop(vm);
                if (IS_ITERABLE_ON(to_get_iter)) {
                    return runtime_error(vm, "value is not iterable", ERR_TYPE);
                }
                IterableObj* iter_obj = get_iterable(AS_OBJ(to_get_iter));

                Value f_value = VAR_OBJ(iter_obj);
                if (IS_STRING(f_value)) { // Check if string, Strings are immutable objects. therefore f_value will contain a copy which should be garbage collected when can.
                    add_garbage(vm, f_value);
                }
                push(vm, f_value);
                break;
            }
            case OP_END_FOR: {
                Value iter_obj = pop(vm);
                // Free the iter_obj as it is not useful anymore
                free_object(AS_OBJ(iter_obj));
                break;
            }
            case OP_FOR_ITER: {
                Value val = peek_behind(vm, 1);
                if (!IS_ITERABLE(val)) {
                    return runtime_error(vm, "expected iterable", ERR_TYPE);
                }
                IterableObj* iter_obj = AS_ITERABLE(val);
                if (iterable_out_of_bounds(iter_obj)) {
                    frame->ip += READ_SHORT();
                    break;
                }
                READ_SHORT();
                Value iterable_var_value = iterable_get_at(iter_obj, iter_obj->index);
                iter_obj->index++;
                if (IS_STRING(iterable_var_value)) { // Add string to garbage because it is immutable. therefore, a copy string is made.
                    add_garbage(vm, iterable_var_value);
                }
                push(vm, iterable_var_value);
                break;
            }
			case OP_CALL: {
                uint8_t arg_count = READ_BYTE();
                Value func_value = peek_behind(vm, arg_count + 1);

                if (IS_NATIVE(func_value)) {
                    NativeFuncObj* native_obj = AS_NATIVE(func_value);
                    Value return_value = native_obj->function(arg_count, vm->sp - arg_count);
                    vm->sp -= arg_count - 1;
                    push(vm, return_value);
                    break;
                }

                if (IS_NATIVE_METHOD(func_value)) {
                    Value func_host = peek_behind(vm, arg_count + 2);
                    NativeFuncObj* native_obj = AS_NATIVE(func_value);
                    Value return_value = native_obj->function(arg_count + 1, &func_host);
                    vm->sp -= arg_count + 2;
                    push(vm, return_value);
                    break;
                }


                if (!IS_FUNCTION(func_value)) {
                    return runtime_error(vm, "object is not callable", ERR_NAME);
                }
                StackFrame func_frame;
                func_frame.function = AS_FUNCTION(func_value);
                func_frame.ip = func_frame.function->body.codes;

                push_frame(vm, func_frame);

                for (uint8_t i = arg_count; i >0; i--) {
                    Value curr_arg = pop(vm);
                    func_frame.function->locals[i - 1].value = curr_arg;
                }
                pop(vm);
                frame = &vm->callStack[vm->frameCount - 1];
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

