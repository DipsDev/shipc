#include "chunk.h"
#include "memory.h"

void init_chunk(Chunk* chunk) {
	chunk->capacity = 0;
	chunk->count = 0;
	chunk->codes = NULL;
	ValueArray arr;
	init_value_array(&arr);
	chunk->constants = arr;
	
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

int add_constant(Chunk* chunk, Value constant) {
	write_value_array(&chunk->constants, constant);
	return chunk->constants.count - 1;
}

void free_chunk(Chunk* chunk) {
	FREE_ARRAY(uint8_t, chunk->codes, chunk->capacity);
	free_value_array(&chunk->constants);
	init_chunk(chunk);
}