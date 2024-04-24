#pragma once
#ifndef SHIP_VM_H_
#define SHIP_VM_H_

#include <stdint.h>
#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef enum {
	OP_ADD,
	OP_CONSTANT,
	OP_HALT
} OpCode;


typedef struct {
	Chunk* chunk;
	uint8_t* ip;

	Value stack[STACK_MAX];
	Value* stack_top;
} VM;


void init_vm(VM* vm);
void set_vm_chunk(VM* vm, Chunk* chunk);
void interpret(VM* vm);
void free_vm(VM* vm);

#endif // !SHIP_VM_H
