#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "objects.h"
#include "value.h"
#include "vm.h"

static Obj* allocate_object(size_t size, ObjType type) {
    Obj* c_obj = (Obj*) malloc(size);
    c_obj->type = type;
    c_obj->isMarked = false;
    c_obj->next = NULL;
    return c_obj;
}

#define ALLOCATE_OBJECT(Obj, type) \
    ((Obj*) allocate_object(sizeof(Obj), type))

// <---- freeing related functions ----->
static void free_string(Obj* str_obj) {
	StringObj* obj = (StringObj*)str_obj;
	free(obj->value);
	free(obj);
}

static void free_native(Obj* str_obj) {
    NativeFuncObj* native = (NativeFuncObj*) str_obj;
    free(native);
}

static void free_function(Obj* func_obj) {
	FunctionObj* obj = (FunctionObj*)func_obj;

	free_string((Obj *) obj->name);
	free_chunk(&obj->body);
    free(obj);

}

static void free_array(Obj* arr_obj) {
    ArrayObj* obj = (ArrayObj*) arr_obj;
    free_value_array(obj->values);
    free(obj);
}

static void free_error(Obj* err_obj) {
    ErrorObj* obj = (ErrorObj*) err_obj;
    free_string((Obj *) obj->value);
    free(obj);


}

static void free_iterable(Obj* iter_obj) {
    IterableObj* obj = (IterableObj*) iter_obj;

    // Don't free the iterable object, the object is not a copy of the original. therefore, it can still be marked.
    // free_object(obj->iterable);

    free(obj);
}

void free_object(Obj* obj) {
	switch (obj->type) {
	case OBJ_STRING: return free_string(obj);
	case OBJ_FUNCTION: return free_function(obj);
    case OBJ_ERROR: return free_error(obj);
    case OBJ_ITERABLE: return free_iterable(obj);
    case OBJ_ARRAY: return free_array(obj);
    case OBJ_NATIVE_METHOD:
    case OBJ_NATIVE: return free_native(obj);
	default: printf("[ERROR] cannot free object, it is not yet supported. got object %d", obj->type); // unreachable
	}
}
// <------------------------------------>


// <---- compare related functions ----->
static bool compare_strings(StringObj* a, StringObj* b) {
	if (a->length != b->length) {
		return false;
	}
	return memcmp(a->value, b->value, a->length) == 0;

}


bool compare_objects(Obj* obj1, Obj* obj2) {
	if (obj1->type != obj2->type) {
		return false;
	}
	switch (obj1->type) {
	case OBJ_STRING: return compare_strings((StringObj*) obj1, (StringObj*) obj2); // == between strings returns if the value of the strings is equal
        default:return obj1 == obj2; // == between other objects will be true only if their memory addresses are the same.
	}
}
bool iterable_out_of_bounds(IterableObj * iterable) {
    switch (iterable->iterable->type) {
        case OBJ_STRING: {
            StringObj* temp_obj = (StringObj*) iterable->iterable;
            return temp_obj->length <= iterable->index;

        } case OBJ_ARRAY: {
                ArrayObj* temp_obj = (ArrayObj* )iterable->iterable;
                return iterable->index >= temp_obj->values->count;
        }
        default: return true; // Add more as the vm gets bigger

    }
}

static Value copy_value(Value val) {
    if (!IS_OBJ(val)) return val;
    Obj* obj_val = AS_OBJ(val);
    switch (obj_val->type) {
        case OBJ_STRING: {
            StringObj* string_obj = CONVERT_OBJ(StringObj, obj_val);
            return VAR_OBJ(create_string_obj(string_obj->value, string_obj->length));
        }
        default: return val;
    }
}

Value iterable_get_at(IterableObj* iterable, int index) {
    switch(iterable->iterable->type) {
        case OBJ_STRING: {
            StringObj* string_obj = CONVERT_OBJ(StringObj, iterable->iterable);
            StringObj* val_obj = create_string_obj(string_obj->value + index, 1);
            return VAR_OBJ(val_obj);
        }
        case OBJ_ARRAY: {
            ArrayObj* arr_obj = (ArrayObj*) iterable->iterable;
            return copy_value(arr_obj->values->arr[index]);
        }
        default: return VAR_NIL;
    }
}
// <------------------------------------>

static char* copy_string(const char* value, int length) {
    char *str_value = (char *) malloc(length + 1);
    if (str_value == NULL) {
        printf("[ERROR] cannot allocate string. exiting...\n");
        exit(1);
    }
    memcpy(str_value, value, length);
    str_value[length] = '\0';
    return str_value;
}

StringObj* create_string_obj(const char* value, int length) {
	// create the required arguments
	char* string_value = copy_string(value, length);
	StringObj* str_obj = ALLOCATE_OBJECT(StringObj, OBJ_STRING);
	if (str_obj == NULL) {
		printf("[ERROR] couldn't allocate string object");
		exit(1);
	}

	// set the values
	str_obj->value = string_value;
	str_obj->length = length;

	return str_obj;
}


ErrorObj* create_err_obj(const char* value, int length, ErrorType type) {
    StringObj* name = create_string_obj(value, length);
    ErrorObj* err_obj = ALLOCATE_OBJECT(ErrorObj, OBJ_ERROR);
    if (err_obj == NULL) {
        printf("[ERROR] couldn't allocate error object");
        exit(1);
    }
    err_obj->value = name;
    err_obj->type = type;
    return err_obj;
}


ArrayObj* create_array_obj() {
    ArrayObj* arr = ALLOCATE_OBJECT(ArrayObj, OBJ_ARRAY);
    arr->values = malloc(sizeof(ValueArray));
    init_value_array(arr->values);
    return arr;
}


FunctionObj* create_func_obj(const char* value, int length, FunctionType type) {
	// create the required arguments
	StringObj* name = create_string_obj(value, length);
	FunctionObj* func_obj = ALLOCATE_OBJECT(FunctionObj, OBJ_FUNCTION);
	if (func_obj == NULL) {
		printf("[ERROR] couldn't allocate string object");
		exit(1);
	}

	// set the values
	func_obj->name = name;
    func_obj->type = type;

	Chunk body;
	init_chunk(&body);
	func_obj->body = body;


	return func_obj;
}
NativeFuncObj* create_native_func_obj(NativeFn function) {
    NativeFuncObj* func_obj = ALLOCATE_OBJECT(NativeFuncObj, OBJ_NATIVE);
    func_obj->function = function;
    return func_obj;
}

NativeFuncObj* create_native_method_obj(NativeFn function) {
    NativeFuncObj* func_obj = ALLOCATE_OBJECT(NativeFuncObj, OBJ_NATIVE_METHOD);
    func_obj->function = function;
    return func_obj;
}

IterableObj* get_iterable(Obj* iterable) {
    IterableObj* iter_obj = ALLOCATE_OBJECT(IterableObj, OBJ_ITERABLE);
    iter_obj->index = 0;
    iter_obj->iterable = iterable;
    return iter_obj;
}


StringObj* concat_strings(const char* value1, int length1, const char* value2, int length2) {
	// create the required strings
	char* string_value = (char*)malloc(length1 + length2 + 1);
	if (string_value == NULL) {
		printf("[ERROR] couldn't allocate string object");
		exit(1);
	}

	// copy the data to the correct places
	memcpy(string_value, value1, length1);
	memcpy(string_value + length1, value2, length2);
	string_value[length1 + length2] = '\0';

	StringObj* str_obj = ALLOCATE_OBJECT(StringObj, OBJ_STRING);
	if (str_obj == NULL) {
		printf("[ERROR] couldn't allocate string object");
		exit(1);
	}

	str_obj->value = string_value;
	str_obj->length = length1 + length2;
	return str_obj;

}