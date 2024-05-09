#pragma once
#ifndef SHIP_VM_H_
#define SHIP_VM_H_

#include <stdint.h>


#include "value.h"
#include "chunk.h"
#include "table.h"
#include "objects.h"

#define STACK_MAX 512

typedef struct {
	// pointers
	uint8_t* ip; // instruction pointer
	Value* sp; // stack pointer

	// objects
	Value stack[STACK_MAX]; // value stack
	Chunk* chunk;
	HashMap* globals;

} VM;

void init_vm(VM* vm);
void free_vm(VM* vm);
void interpret(VM* vm, FunctionObj* script);

#endif // !SHIP_VM_H_
