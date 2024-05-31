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

static void pop_free_head(Obj ** head) {
    Obj* next_node = NULL;

    if (*head == NULL) {
        printf("pop_free_head got empty list");
        return;
    }
    next_node = (Obj *) (*head)->next;

    free_object(*head);

    *head = next_node;

}

static void pop_free_next(Obj ** prev) {
    Obj* aim_node = (Obj *) (*prev)->next;
    (*prev)->next = (struct Obj *) aim_node->next;
    free_object(aim_node);

}

static void sweep(VM* vm) {
    Obj* prev = NULL;
    Obj* pos = (Obj *) vm->objects;
    int before_objs = vm->heapObjects;
    while (pos != NULL) {
        if (pos->isMarked) {
            pos->isMarked = false;
            prev = pos;
            pos = (Obj *) pos->next;
            continue;
        }
        if (prev == NULL) {
            pop_free_head((Obj **) &vm->objects);
            prev = (Obj *) vm->objects;
            pos = (Obj *) prev->next;
        } else {
            pop_free_next(&prev);
            pos = (Obj*) prev->next;
        }
        vm->heapObjects--;
    }
    printf("GC: Finished cleaning %i objects\n", before_objs - vm->heapObjects);
}

void add_garbage(VM* vm, Value value) {
    if (!IS_OBJ(value)) return;
    Obj *const_obj = AS_OBJ(value);
    // append the new obj to the head of the list
    Obj **head = (Obj **) &vm->objects;

    const_obj->next = (struct Obj *) *head;
    *head = const_obj;

    vm->heapObjects++;
}

void collect_garbage(VM* vm) {
    // don't gc if the capacity isn't overflowing
    if (vm->heapObjects + 1 < vm->heapCapacity) return;
    // mark and sweep
    mark_variables(vm);
    sweep(vm);

    // bop the capacity to reduce collection
    vm->heapCapacity *= 2;

}
