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

static unsigned hash_function(char* name, int length) {
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
			put_node_t(pos->name, strlen(pos->name), new_capacity, temp_, pos);
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
	// free each bucket, and then free the entire map
	for (unsigned int i = 0; i < map->capacity; i++) {
		if (map->arr[i] == NULL) {
			continue;
		}
		HashNode* pos = map->arr[i];
		while (pos != NULL) {
			HashNode* before = (HashNode *) pos->next;
            free(pos->name);
			free(pos);
			pos = before;
		}
	}
    free(map->arr);
	free(map);
}


