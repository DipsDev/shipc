#pragma once
#ifndef SHIP_VM_H_
#define SHIP_VM_H_

#include <stdint.h>
#include "chunk.h"

typedef enum {
	OP_ADD,
	OP_CONSTANT,
	OP_HALT
} OpCode;

typedef struct {
	Chunk* chunk;
	uint8_t* ip;
} VM;


void init_vm(VM* vm);
void set_vm_chunk(VM* vm, Chunk* chunk);
void interpret(VM* vm);

#endif // !SHIP_VM_H
