#pragma once
#ifndef SHIP_VM_H_
#define SHIP_VM_H_

#include <stdint.h>


#include "value.h"
#include "chunk.h"

#define STACK_MAX 512

typedef struct {
	// pointers
	uint8_t* ip; // instruction pointer
	Value* sp; // stack pointer

	// objects
	Value stack[STACK_MAX]; // value stack
	Chunk* chunk;

} VM;

#endif // !SHIP_VM_H_
