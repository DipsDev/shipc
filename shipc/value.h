#pragma once
#ifndef SHIP_VALUE_H_
#define SHIP_VALUE_H_


#include <stdbool.h>

typedef enum {
	VAL_NIL,
	VAL_BOOL,
	VAL_NUMBER,
	VAL_OBJ,
} ValueType;

typedef enum {
	OBJ_STRING,
} ObjType;

typedef struct {
	ObjType type;
} Obj;

/// Object Types
typedef struct {
	Obj* obj;
	char* value;
	int length;
} StringObj;
/// 

typedef struct {
	ValueType type;
	union {
		double number;
		bool boolean;
		Obj* obj;
	} as;
} Value;


typedef struct {
	int count;
	int capacity;
	Value* arr;
} ValueArray;

void write_value_array(ValueArray* values, Value value);
void init_value_array(ValueArray* arr);
void free_value_array(ValueArray* arr);
void print_value(Value val);
bool is_truthy(Value val);


// fetch from a tagged union
#define AS_NUMBER(value) ((value).as.number)
#define AS_BOOL(value) ((value).as.boolean)
#define AS_OBJ(value) ((value).as.obj)


// create a tagged union
#define VAR_NUMBER(value) ((Value) {VAL_NUMBER, { .number = value}})
#define VAR_BOOL(value) ((Value) { VAL_BOOL, { .boolean = value }})
#define VAR_NIL ((Value) { VAL_NIL, { .number = 0 }})
#define VAR_OBJ(object) ((Value) { VAl_OBJ, { .obj = (Obj*) object}})


// type testing
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#endif // !SHIP_VALUE_H_

