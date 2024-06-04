#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "table.h"
#include "string.h"
#include "memory.h"
#include "objects.h"



void create_variable_map(HashMap* mp) {
	mp->capacity = 8;
	mp->count = 0;
    mp->arr = calloc(mp->capacity, sizeof(HashNode*));
}

static unsigned hash_function(const char* name, int length) {
	unsigned hash = 2166136261;
	int i = 0;
	while (i < length) {
		hash = hash ^ name[i];
		hash = hash * 16777619;
		i++;
	}
	return hash; // & (capacity - 1); calculate the modulo of the capacity
}

static HashNode* create_node(char* name, size_t len, unsigned int val) {
	HashNode* node = (HashNode*)malloc(sizeof(HashNode));
	if (node == NULL) {
		printf("Failed to allocate node");
		exit(1);
	}
	node->name = name;
    node->len = len;
	node->value = val;
	node->next = NULL;
    return node;
}

static void put_node_t(char* name, int name_len, int capacity, HashNode** arr, HashNode* nd) {
	unsigned index = hash_function(name, name_len) & (capacity - 1);
	if (arr[index] == NULL) {
		arr[index] = nd;
		return;
	}
	// move to the end of the linked list
	HashNode* pos = arr[index];
	while (pos->next != NULL) {
		pos = pos->next;
	}
	pos->next = nd;
}

static void resize_map(HashMap* map) {
	// create new capacity
	int new_capacity = GROW_CAPACITY(map->capacity);
	HashNode** temp_ = calloc(new_capacity, sizeof(HashNode*));

	// recalculate the values
	for (unsigned int i = 0; i < map->capacity; i++) {
		if (map->arr[i] == NULL) {
			continue;
		}
		HashNode* pos = map->arr[i];
		while (pos != NULL) {
			put_node_t(pos->name, pos->len, new_capacity, temp_, pos);
			pos = (HashNode *) pos->next;
		}
	}

	// free and assign the new array
	free(map->arr);
	map->arr = temp_;

}

void put_node(HashMap* map,char* name,int name_len, unsigned int val) {
	HashNode* nd = create_node(name, name_len, val);
	if ((double)map->count / map->capacity >= 0.75) {
		resize_map(map);
	}
    map->count++;
	put_node_t(name, name_len, map->capacity, map->arr, nd);
}

HashNode* get_node(HashMap* map, char* name, int name_len) {
	unsigned index = hash_function(name, name_len) & (map->capacity - 1); // calculate the index
	HashNode* pos = map->arr[index];
	while (pos != NULL && strncmp(name, pos->name, pos->len) != 0) {
		pos = pos->next;
	}
	return pos;
}

void free_hash_map(HashMap* map) {
    free(map->arr); // Free the array directly, as the char* is right from the source code.
	free(map);
}


void create_value_map(ValueTable * mp) {
    mp->capacity = 8;
    mp->count = 0;
    mp->arr = calloc(mp->capacity, sizeof(ValueNode *));
}

ValueNode * create_value_node(char* name, int len, Value val) {
    ValueNode * node = (ValueNode *) malloc(sizeof(ValueNode));
    if (node == NULL) {
        printf("Failed to allocate node");
        exit(1);
    }
    node->name = name;
    node->length = len;
    node->val = val;
    node->next = NULL;
    return node;
}
static void put_value_node_t(char* name, int name_len, int capacity, ValueNode ** arr, ValueNode * nd) {
    unsigned index = hash_function(name, name_len) & (capacity - 1);
    if (arr[index] == NULL) {
        arr[index] = (ValueNode *) nd;
        return;
    }
    // move to the end of the linked list
    ValueNode *pos = (ValueNode *) arr[index];
    while (pos->next != NULL) {
        pos = (ValueNode *) pos->next;
    }
    pos->next = (struct ValueNode *) nd;
}

static void resize_value_table(ValueTable *map) {
    // create new capacity
    int new_capacity = GROW_CAPACITY(map->capacity);
    ValueNode ** temp_ = (ValueNode **) calloc(new_capacity, sizeof(ValueNode *));

    // recalculate the values
    for (unsigned int i = 0; i < map->capacity; i++) {
        if (map->arr[i] == NULL) {
            continue;
        }
        ValueNode * pos = map->arr[i];
        while (pos != NULL) {
            put_value_node_t(pos->name, pos->length, new_capacity, temp_, pos);
            pos = (ValueNode *) pos->next;
        }
    }

    // free and assign the new array
    free(map->arr);
    map->arr = temp_;

}


void put_value_node(ValueTable * map,char* name,int name_len, Value val) {
    ValueNode * nd = create_value_node(name, name_len, val);
    if ((double)map->count / map->capacity >= 0.75) {
        resize_value_table((ValueTable *) map);
    }
    map->count++;
    put_value_node_t(name, name_len, map->capacity, map->arr, nd);
}

ValueNode * get_global(ValueTable * map, char* name, int name_len) {
    unsigned index = hash_function(name, name_len) & (map->capacity - 1); // calculate the index
    ValueNode * pos = map->arr[index];
    while (pos != NULL && strncmp(name, pos->name, pos->length) != 0) {
        pos = (ValueNode *) pos->next;
    }
    return pos;
}

void free_globals(ValueTable * map) {
    // free each bucket, and then free the entire map
    for (unsigned int i = 0; i < map->capacity; i++) {
        if (map->arr[i] == NULL) {
            continue;
        }
        ValueNode * pos = (ValueNode *) map->arr[i];
        while (pos != NULL) {
            ValueNode * before = (ValueNode *) pos->next;
            if (IS_OBJ(pos->val) && !IS_NATIVE(pos->val)) { // Don't free the native functions.
                free_object(AS_OBJ(pos->val));
                free(pos->name);
            }
            free(pos);
            pos = before;
        }
    }
    free(map->arr);
}