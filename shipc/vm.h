#pragma once
#ifndef SHIP_VM_H_
#define SHIP_VM_H_

#include <stdint.h>


#include "value.h"
#include "chunk.h"
#include "table.h"
#include "objects.h"

#define STACK_MAX 512
#define CALL_STACK_MAX 512


typedef enum {
    RESULT_SUCCESS,
    RESULT_ERROR
} InterpretResult;

typedef struct {
    FunctionObj* function;
    uint8_t* ip;
} StackFrame;

typedef struct {
	// pointers
	Value* sp; // stack pointer

    unsigned int frameCount;
    StackFrame callStack[CALL_STACK_MAX];

	// objects
	Value stack[STACK_MAX]; // value stack

} VM;

void init_vm(VM* vm);
void free_vm(VM* vm);
InterpretResult interpret(VM* vm, FunctionObj* main_script);

#endif // !SHIP_VM_H_
