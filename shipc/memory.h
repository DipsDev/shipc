#pragma once
#ifndef SHIP_MEMORY_H_
#define SHIP_MEMORY_H_

#include "value.h"
#include "vm.h"

#define GROW_CAPACITY(capacity) \
	(capacity >= 8 ? capacity * 2 : 8)
#define GROW_ARRAY(type, arr, oldCapacity, newCapacity) \
	((type*) reallocate(arr, oldCapacity * sizeof(type), newCapacity * sizeof(type))) \

#define FREE_ARRAY(type, arr, oldCapacity) \
	((type*) reallocate(arr, oldCapacity * sizeof(type), 0)) \



void* reallocate(void* pointer, size_t oldCapacity, size_t newCapacity);

// Garbage collector related
void mark_value(Value value);
void mark_object(Obj* obj);

void collect_garbage(VM* vm);

#endif // SHIP_MEMORY_H_
