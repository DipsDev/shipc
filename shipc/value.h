#pragma once
#ifndef SHIP_VALUE_H_
#define SHIP_VALUE_H_


typedef double Value;



#define NUMBER(value) (Value) (value)
#define BINARY_OP(a, op, b) \
	(Value) (a op b) \


#endif // !SHIP_VALUE_H_

