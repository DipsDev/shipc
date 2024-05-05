#pragma once
#ifndef SHIP_OBJECTS_H
#define SHIP_OBJECTS_H

#include "value.h"

StringObj* create_string_obj(const char* value, int length);
StringObj* concat_strings(const char* value1, int length1, const char* value2, int length2);
bool compare_objects(Obj* obj1, Obj* obj2);

#endif // !SHIP_OBJECTS_H










