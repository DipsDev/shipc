#include <stdlib.h>

#include "memory.h"


void* reallocate(void* pointer, size_t oldCapacity, size_t newCapacity) {
	if (newCapacity == 0) {
		free(pointer);
		return NULL;
	}

	void* result = realloc(pointer, newCapacity);
	return result;
}

void mark_object(Obj* obj) {
    if (obj == NULL) return;
    obj->marked = true;
}

void mark_value(Value value) {
    if (!IS_OBJ(value)) return;
    mark_object(AS_OBJ(value));

}

void mark_vm(VM* vm) {

}
