#pragma once
#ifndef SHIP_OBJECTS_H_
#define SHIP_OBJECTS_H_

#include "value.h"
#include "chunk.h"

/// Object Types
typedef struct {
	Obj obj;
	char* value;
	int length;
} StringObj;


typedef enum {
    FN_SCRIPT,
    FN_FUNCTION
} FunctionType;

typedef struct {
	Obj obj;
	Chunk body;
	StringObj* name;
    FunctionType type;
} FunctionObj;

// error related enums
typedef enum {
    ERR_SYNTAX,
    ERR_NAME,
    ERR_TYPE
} ErrorType;

typedef struct {
    Obj obj;
    StringObj* value;
    ErrorType type;

} ErrorObj;
/// 

StringObj* create_string_obj(const char* value, int length);
StringObj* concat_strings(const char* value1, int length1, const char* value2, int length2);

FunctionObj* create_func_obj(const char* value, int length, FunctionType type);


ErrorObj* create_err_obj(const char* value, int length, ErrorType type);

void free_object(Obj* obj);
bool compare_objects(Obj* obj1, Obj* obj2);

#endif // !SHIP_OBJECTS_H_










