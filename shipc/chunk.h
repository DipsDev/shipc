#pragma once
#ifndef SHIP_CHUNK_H_
#define SHIP_CHUNK_H_

#include <stdint.h>

typedef enum { // available op codes
	OP_HALT
} OpCode;


typedef struct { // store a chunk of bytecode
	uint8_t* codes; // op codes / values
	int count; // currently active elements
	int capacity; // available total capacity
} Chunk;


void init_chunk(Chunk* chunk);
void free_chunk(Chunk* chunk);
void write_chunk(Chunk* chunk, uint8_t byte);

#endif // SHIP_CHUNK_H_
