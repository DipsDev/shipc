#include <stdio.h>

#include "memory.h"
#include "value.h"
#include "objects.h"

bool is_truthy(Value val) {
	if (IS_NUMBER(val)) {
		return AS_NUMBER(val) != 0;
	}
	if (IS_BOOL(val)) {
		return AS_BOOL(val);
	}
	return false;
}


void init_value_array(ValueArray* arr) {
	arr->capacity = 0;
	arr->count = 0;
	arr->arr = NULL;
}

void write_value_array(ValueArray* values, Value value) {
	if (values->capacity <= values->count + 1) {
		int oldCapacity = values->capacity;
		values->capacity = GROW_CAPACITY(oldCapacity);
		values->arr = GROW_ARRAY(Value, values->arr, oldCapacity, values->capacity);
	}
	values->arr[values->count] = value;
	values->count++;
}

void free_value_array(ValueArray* arr) {
	FREE_ARRAY(Value, arr->arr, arr->capacity);
	init_value_array(arr);
}

static void print_object(Obj* obj) {
	switch (obj->type) {
	case OBJ_STRING: printf("%s", ((StringObj*) obj)->value);
	}
}

void print_value(Value val) {
	switch (val.type) {
	case VAL_BOOL: printf("%s", AS_BOOL(val) ? "true" : "false"); break;
	case VAL_NIL: printf("nil"); break;
	case VAL_NUMBER: printf("%f", AS_NUMBER(val)); break;
	case VAL_OBJ: print_object(AS_OBJ(val)); break;
	}
}

