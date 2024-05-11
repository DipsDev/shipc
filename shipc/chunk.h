#pragma once
#ifndef SHIP_CHUNK_H_
#define SHIP_CHUNK_H_

#include <stdint.h>
#include "value.h"

typedef enum { // available op codes
	OP_CONSTANT,
	OP_MUL,
	OP_POP_TOP,
	OP_FALSE,
	OP_TRUE,
	OP_CALL,
	OP_NIL,
	OP_ADD,
	OP_SUB,
	OP_STORE_FAST,
    OP_LOAD_LOCAL,
	OP_LOAD_GLOBAL,
	OP_ASSIGN_GLOBAL,
	OP_DIV,
    OP_RETURN,
	OP_SHOW_TOP,
	OP_COMPARE,
	OP_NEGATE,
	OP_POP_JUMP_IF_FALSE,
	OP_NOT,
	OP_HALT
} OpCode;


typedef struct { // store a chunk of bytecode
	uint8_t* codes; // op codes / values
	int count; // currently active elements
	int capacity; // available total capacity

	ValueArray constants; // constant pool
} Chunk;


void init_chunk(Chunk* chunk);
void free_chunk(Chunk* chunk);
void write_chunk(Chunk* chunk, uint8_t byte);
void write_bytes(Chunk* chunk, uint8_t byte, uint8_t byte2);
uint8_t add_constant(Chunk* chunk, Value constant);
void change_constant(Chunk* chunk, uint8_t index, Value constant);

#endif // SHIP_CHUNK_H_
