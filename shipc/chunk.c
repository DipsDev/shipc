#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "chunk.h"


void init_chunk(Chunk* chunk) {
	chunk->capacity = 0;
	chunk->count = 0;
	chunk->codes = NULL;
}

void add_code(Chunk* chunk, uint8_t code) {
	// if the total elements will be larger than the capacity
	if (chunk->count + 1 > chunk->capacity) {
		// grow the array
		int new_capacity = chunk->capacity > 8 ? chunk->capacity * 2 : 8;
		uint8_t* new_codes = (uint8_t*) realloc(chunk->codes, new_capacity * sizeof(uint8_t));
		if (new_codes == NULL) {
			printf("Ran out of memory");
			exit(1);
		}
		chunk->codes = new_codes;
	}
	chunk->codes[chunk->count] = code;
	chunk->count++;
}