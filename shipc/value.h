#pragma once
#ifndef SHIP_VALUE_H_
#define SHIP_VALUE_H_

#define STACK_MAX 256

typedef double Value;

typedef struct {
	Value values[STACK_MAX];
	int current;
} StackValue;


void init_value_stack(StackValue* stack);
Value pop_value_stack(StackValue* stack);

#endif // !SHIP_VALUE_H_
