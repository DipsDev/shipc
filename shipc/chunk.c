#include "chunk.h"
#include "memory.h"

void init_chunk(Chunk* chunk) {
	chunk->capacity = 0;
	chunk->count = 0;
	chunk->codes = NULL;
}

void write_chunk(Chunk* chunk, uint8_t byte) {
	if (chunk->capacity < chunk->count + 1) {
		int oldCapacity = chunk->capacity;
		chunk->capacity = GROW_CAPACITY(oldCapacity);
		chunk->codes = GROW_ARRAY(uint8_t, chunk->codes, oldCapacity, chunk->capacity);
	}
	chunk->codes[chunk->count] = byte;
	chunk->count++;
}

void free_chunk(Chunk* chunk) {
	FREE_ARRAY(uint8_t, chunk->codes, chunk->capacity);
	init_chunk(chunk);
}