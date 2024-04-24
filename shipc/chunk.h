#pragma once
#ifndef SHIP_CHUNK_H_
#define SHIP_CHUNK_H_

#include <stdint.h>

typedef struct {
	uint8_t* codes;
	int capacity;
	int count;
} Chunk;

void add_code(Chunk* chunk, uint8_t code);
void init_chunk(Chunk* chunk);
void print_chunk(Chunk* chunk);

#endif // !SHIP_CHUNK_H_
