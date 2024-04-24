#include <stdio.h>

#include "value.h"
#include "vm.h"

void push(Value* stack, Value* sp, Value value) {
	if ((size_t)(sp - stack) == STACK_MAX) {
		printf("Stack overflow");
		exit(1);
	}
	*sp = value;
	sp++;
}

Value pop(Value* stack, Value* sp) {
	if (stack == sp) {
		printf("Empty stack");
		exit(1);
	}
	sp--;
	return *sp;
}