#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

static void print_object(Value obj_val) {
	switch (AS_OBJ(obj_val)->type) {
	case OBJ_STRING: {
        StringObj* str_obj = AS_STRING(obj_val);
        for (int i = 0; i < str_obj->length -1; i++) {
            if (str_obj->value[i] == '\\' && str_obj->value[i + 1] == 'n') {
                str_obj->value[i] = '\r';
                str_obj->value[i + 1] = '\n';
            }

        }
        printf("%.*s", str_obj->length, str_obj->value);
        break;
    }
        case OBJ_FUNCTION: {
            FunctionObj *func = AS_FUNCTION(obj_val);
            printf("<function %.*s at %p>", func->name->length, func->name->value, func);
            break;
        }
        case OBJ_ERROR:
            break;
    }
}

void print_value(Value val) {
	switch (val.type) {
	case VAL_BOOL: printf("%s", AS_BOOL(val) ? "true" : "false"); break;
	case VAL_NIL: printf("nil"); break;
	case VAL_NUMBER:  {
        double dummy;
        if (modf(AS_NUMBER(val), &dummy) == 0) {
            printf("%.0f", AS_NUMBER(val)); break;
            return;
        }
        printf("%f", AS_NUMBER(val)); break;
    }
	case VAL_OBJ: print_object(val); break;
	}
}

