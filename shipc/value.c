#include <stdio.h>

#include "memory.h"
#include "value.h"

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

