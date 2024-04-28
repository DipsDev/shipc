#pragma once
#ifndef SHIP_MEMORY_H_
#define SHIP_MEMORY_H_

#define GROW_CAPACITY(capacity) \
	(capacity >= 8 ? capacity * 2 : 8)
#define GROW_ARRAY(type, arr, oldCapacity, newCapacity) \
	((type*) reallocate(arr, oldCapacity * sizeof(type), newCapacity * sizeof(type))) \

#define FREE_ARRAY(type, arr, oldCapacity) \
	((type*) reallocate(arr, oldCapacity * sizeof(type), 0)) \



void* reallocate(void* pointer, size_t oldCapacity, size_t newCapacity);

#endif // SHIP_MEMORY_H_
