#pragma once
#ifndef SHIP_VALUE_H_
#define SHIP_VALUE_H_


typedef double Value;



#define NUMBER(value) (Value) (value)


void push(Value* stack, Value* sp, Value value);
Value pop(Value* stack, Value* sp);

#endif // !SHIP_VALUE_H_

