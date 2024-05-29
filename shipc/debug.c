#include <stdio.h>

#include "debug.h"
#include "objects.h"




static int simple_instruction(const char* string, int offset) {
	printf("| %04d  %s  |\n", offset, string);
	return 1;
}

static int byte_instruction(Chunk* chunk, const char* string, int offset) {
    printf("| %04d %s (%u) |\n", offset, string, chunk->codes[offset + 1]);
    return 2;
}

static void print_obj(Chunk* chunk, Value val, int offset, char* message) {
	uint8_t i = chunk->codes[offset + 1];
	switch (AS_OBJ(val)->type) {
        case OBJ_STRING: {
            StringObj *obj = AS_STRING(val);
            printf("| %04d %s %u (%.*s) |\n", offset, message, i , obj->length, obj->value);
            break;
        }
        case OBJ_FUNCTION: {
            FunctionObj *obj = AS_FUNCTION(val);
            disassemble_func(obj);
            break;
        }
        case OBJ_ERROR:
            break;
    }
}

static int variable_instruction(FunctionObj * func, char* op_code, int offset) {
	uint8_t index = func->body.codes[offset + 1];
    Local local_var = func->locals[index];
    printf("| %04d %s %u (%.*s) |\n", offset, op_code, index, local_var.length, local_var.name);

	return 2;
}

static int global_variable_instruction(FunctionObj* func, char* op_code, int offset) {
    uint8_t index = func->body.codes[offset + 1];
    Value var = func->body.constants.arr[index];
    printf("| %04d %s %u (%.*s)\n", offset, op_code, index, AS_STRING(var)->length, AS_STRING(var)->value);
    return 2;
}

static int constant_instruction(Chunk* chunk, int offset) {
	uint8_t index = chunk->codes[offset + 1];
	Value val = chunk->constants.arr[index];
	switch (val.type) {
	case VAL_BOOL: printf("| %04d OP_CONSTANT %u (%s) |\n", offset, index, AS_BOOL(val) ? "true" : "false"); break;
	case VAL_NIL: printf("| %04d OP_CONSTANT %u (nil) |\n", offset, index); break;
	case VAL_NUMBER: printf("| %04d OP_CONSTANT %u (%.2f) |\n", offset, index, AS_NUMBER(val)); break;
	case VAL_OBJ: print_obj(chunk, val, offset, "OP_CONSTANT");
	}
	return 2;
}

static int jump_instruction(Chunk* chunk, int offset) {
	uint8_t d1 = chunk->codes[offset + 1];
	uint8_t d2 = chunk->codes[offset + 2];
	printf("| %04d OP_JUMP (%u) |\n", offset, (((uint16_t)d1 << 8) | d2));
	return 3;
}

static int disassemble_instruction(FunctionObj * func, int offset) {
	uint8_t code = func->body.codes[offset];
	switch (code) {
		case OP_HALT: return simple_instruction("OP_HALT", offset);
        case OP_LOAD_LOCAL: return variable_instruction(func, "OP_LOAD_LOCAL", offset);
		case OP_NEGATE: return simple_instruction("OP_NEGATE", offset);
		case OP_ADD: return simple_instruction("OP_ADD", offset);
		case OP_SUB: return simple_instruction("OP_SUB", offset);
		case OP_DIV: return simple_instruction("OP_DIV", offset);
		case OP_MUL: return simple_instruction("OP_MUL", offset);
        case OP_LESS_THAN: return simple_instruction("OP_LESS_THAN", offset);
        case OP_GREATER_THAN: return simple_instruction("OP_GREATER_THAN", offset);
		case OP_FALSE: return simple_instruction("OP_FALSE", offset);
		case OP_CALL: return byte_instruction(&func->body, "OP_CALL", offset);
		case OP_NOT: return simple_instruction("OP_NOT", offset);
        case OP_ASSIGN_LOCAL: return variable_instruction(func, "OP_ASSIGN_LOCAL", offset);
        case OP_ASSIGN_GLOBAL: return global_variable_instruction(func, "OP_ASSIGN_GLOBAL", offset);
		case OP_TRUE: return simple_instruction("OP_TRUE", offset);
		case OP_NIL:return simple_instruction("OP_NIL", offset);
		case OP_STORE_FAST: return variable_instruction(func, "OP_STORE_FAST", offset);
		case OP_LOAD_GLOBAL: return global_variable_instruction(func, "OP_LOAD_GLOBAL", offset);
		case OP_POP_JUMP_IF_FALSE:
        case OP_JUMP_BACKWARD: return jump_instruction(&func->body, offset);
        case OP_SHOW_TOP: return simple_instruction("OP_SHOW_TOP", offset);
		case OP_POP_TOP: return simple_instruction("OP_POP_TOP", offset);
		case OP_COMPARE: return simple_instruction("OP_COMPARE", offset);
        case OP_RETURN: return simple_instruction("OP_RETURN", offset);
		case OP_CONSTANT: return constant_instruction(&func->body, offset);
        default: {
            printf("Uncaught opcode %u", code);
            return 1;
        }


	}
}

void disassemble_func(FunctionObj* obj) {
	printf("=== disassembled %.*s script l(%i) ===\n", obj->name->length, obj->name->value, obj->body.count);
	for (int i = 0; i < obj->body.count;) {
		i += disassemble_instruction(obj, i);
	}
    printf("=== end function %.*s ===\n", obj->name->length, obj->name->value);
}