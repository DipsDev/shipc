#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "objects.h"


static char* allocate_string(const char* value, int length) {
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
	object->type = type;
	return object;
}

StringObj* create_string_obj(const char* value, int length) {
	// create the required arguements
	char* string_value = allocate_string(value, length);
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