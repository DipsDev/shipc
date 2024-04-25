#pragma once
#ifndef SHIP_VALUE_H_
#define SHIP_VALUE_H_


typedef double Value;


typedef struct {
	int count;
	int capacity;
	Value* arr;
} ValueArray;

void write_value_array(ValueArray* values, Value value);
void init_value_array(ValueArray* arr);
void free_value_array(ValueArray* arr);



#define NUMBER(value) (Value) (value)
#define BINARY_OP(a, op, b) \
	(Value) (a op b) \


#endif // !SHIP_VALUE_H_

