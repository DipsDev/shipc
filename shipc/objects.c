#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "objects.h"

// <---- freeing related functions ----->
static void free_string(Obj* str_obj) {
	StringObj* obj = (StringObj*)str_obj;
	free(obj->value);
	free(obj);
}

static void free_function(Obj* func_obj) {
	FunctionObj* obj = (FunctionObj*)func_obj;

	free_string((Obj *) obj->name);
	free_chunk(&obj->body);
	free(func_obj);

}

static void free_error(Obj* err_obj) {
    ErrorObj* obj = (ErrorObj*) err_obj;
    free_string((Obj *) obj->value);
    free(obj);


}

void free_object(Obj* obj) {
	switch (obj->type) {
	case OBJ_STRING: return free_string(obj);
	case OBJ_FUNCTION: return free_function(obj);
    case OBJ_ERROR: return free_error(obj);
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
	case OBJ_STRING: return compare_strings((StringObj*) obj1, (StringObj*) obj2);
	}
}
// <------------------------------------>

static char* copy_string(const char* value, int length) {
	char* str_value = (char*)malloc(length + 1);
	if (str_value == NULL) {
		printf("[ERROR] cannot allocate string. exiting...\n");
		exit(1);
	}
	memcpy(str_value, value, length);
	str_value[length] = '\0';
	return str_value;
}	

static Obj* allocate_object(ObjType type) {
	Obj* object = (Obj*)malloc(sizeof(Obj));
	if (object == NULL) {
		printf("[ERROR] cannot allocate object. exiting...\n");
		exit(1);
	}
    object->marked = false;
	object->type = type;
	return object;
}

StringObj* create_string_obj(const char* value, int length) {
	// create the required arguements
	char* string_value = copy_string(value, length);
	Obj* object = allocate_object(OBJ_STRING);
	StringObj* str_obj = (StringObj*)malloc(sizeof(StringObj));
	if (str_obj == NULL) {
		printf("[ERROR] couldn't allocate string object");
		exit(1);
	}

	// set the values
	str_obj->value = string_value;
	str_obj->obj = *object;
	str_obj->length = length;

	return str_obj;
}

ErrorObj* create_err_obj(const char* value, int length, ErrorType type) {
    StringObj* name = create_string_obj(value, length);
    Obj* object = allocate_object(OBJ_ERROR);
    ErrorObj* err_obj = (ErrorObj*) malloc(sizeof(ErrorObj));
    if (err_obj == NULL) {
        printf("[ERROR] couldn't allocate error object");
        exit(1);
    }
    err_obj->obj = *object;
    err_obj->value = name;
    err_obj->type = type;
    return err_obj;
}

FunctionObj* create_func_obj(const char* value, int length, FunctionType type) {
	// create the required arguements
	StringObj* name = create_string_obj(value, length);
	Obj* object = allocate_object(OBJ_FUNCTION);
	FunctionObj* func_obj = (FunctionObj*)malloc(sizeof(FunctionObj));
	if (func_obj == NULL) {
		printf("[ERROR] couldn't allocate string object");
		exit(1);
	}

	// set the values
	func_obj->name = name;
	func_obj->obj = *object;
    func_obj->type = type;

	Chunk body;
	init_chunk(&body);
	func_obj->body = body;


	return func_obj;
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

	Obj* object = allocate_object(OBJ_STRING);
	StringObj* str_obj = (StringObj*)malloc(sizeof(StringObj));
	if (str_obj == NULL) {
		printf("[ERROR] couldn't allocate string object");
		exit(1);
	}

	str_obj->value = string_value;
	str_obj->obj = *object;
	str_obj->length = length1 + length2;

	return str_obj;

}