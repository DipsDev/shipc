#include <stdlib.h>
#include <stdio.h>

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
    obj->isMarked = true;
}

void mark_value(Value value) {
    if (!IS_OBJ(value)) return;
    mark_object(AS_OBJ(value));

}

static void mark_variables(VM* vm) {
    for (int i = vm->frameCount - 1; i >= 0; i--) {
        StackFrame* curr = &vm->callStack[i];
        // iterate over the function locals and search for the value
        for(int j = 0; j< curr->function->localCount; j++) {
            Local current_local = curr->function->locals[j];
            mark_value(current_local.value);
        }
    }
}

static void sweep(VM* vm) {
    Obj* prev = NULL;
    Obj* pos = vm->objects;
    while (pos != NULL) {
        if (pos->isMarked) {
            pos->isMarked = false;
            prev = pos;
            pos = (Obj *) pos->next;
        } else {
            if (prev == NULL) {
                vm->objects = (Obj *) pos->next;
            } else {
                prev->next = pos->next;
            }
            Obj* temp = (Obj *) pos->next;
            free_object(pos);
            pos = temp;
        }
    }
}

void collect_garbage(VM* vm) {
    mark_variables(vm);
    sweep(vm);

}
