#include "value.h"
#include <stddef.h>


void init_value_stack(StackValue *stack) {
	stack->current = 0;
}

Value pop_value_stack(StackValue* stack) {
	// assumes stack is not empty

	// decrement current
	stack->current--;

	return stack->values[stack->current];
}

void append_value_stack(StackValue* stack, Value value) {
	stack->values[stack->current] = value;
	stack->current++;
}