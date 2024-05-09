#include <stdio.h>

#include "debug.h"
#include "objects.h"




static int simple_instruction(const char* string, int offset) {
	printf("| %04d  %s  |\n", offset, string);
	return 1;
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
    }
}

static int variable_instruction(Chunk* chunk, char* op_code, int offset) {
	uint8_t index = chunk->codes[offset + 1];
	Value val = chunk->constants.arr[index];
    print_obj(chunk, val, offset, op_code);

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

static int call_instruction(Chunk* chunk, int offset) {
	uint8_t index = chunk->codes[offset + 1];
	Value val = chunk->constants.arr[index];
	StringObj* obj = AS_STRING(val);
	printf("| %04d OP_CALL %.*s\n", offset, obj->length, obj->value);
	return 2;
}

static int disassemble_instruction(Chunk* chunk, int offset) {
	uint8_t code = chunk->codes[offset];
	switch (code) {
		case OP_HALT: return simple_instruction("OP_HALT", offset);
		case OP_NEGATE: return simple_instruction("OP_NEGATE", offset);
		case OP_ADD: return simple_instruction("OP_ADD", offset);
		case OP_SUB: return simple_instruction("OP_SUB", offset);
		case OP_DIV: return simple_instruction("OP_DIV", offset);
		case OP_MUL: return simple_instruction("OP_MUL", offset);
		case OP_FALSE: return simple_instruction("OP_FALSE", offset);
		case OP_CALL: return call_instruction(chunk, offset);
		case OP_NOT: return simple_instruction("OP_NOT", offset);
		case OP_TRUE: return simple_instruction("OP_TRUE", offset);
		case OP_NIL:return simple_instruction("OP_NIL", offset);
		case OP_ASSIGN_GLOBAL: return variable_instruction(chunk, "OP_ASSIGN_GLOBAL", offset);
		case OP_STORE_GLOBAL: return variable_instruction(chunk, "OP_STORE_GLOBAL", offset);
		case OP_LOAD_GLOBAL: return variable_instruction(chunk, "OP_LOAD_GLOBAL", offset);
		case OP_POP_JUMP_IF_FALSE: return jump_instruction(chunk, offset);
		case OP_POP_TOP: return simple_instruction("OP_POP_TOP", offset);
		case OP_COMPARE: return simple_instruction("OP_COMPARE", offset);
		case OP_CONSTANT: return constant_instruction(chunk, offset);
        default: return 1; // should be unreachable


	}
}

void disassemble_func(FunctionObj* obj) {
	printf("=== disassembled %.*s script l(%i) ===\n", obj->name->length, obj->name->value, obj->body.count);
	for (int i = 0; i < obj->body.count;) {
		i += disassemble_instruction(&obj->body, i);
	}
    printf("=== end function %.*s ===\n", obj->name->length, obj->name->value);
}