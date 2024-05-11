#pragma once
#ifndef SHIP_TABLE_H_
#define SHIP_TABLE_H_

#include "value.h"
#include "objects.h"

typedef struct {
	char* name;
    size_t len;
	unsigned int value;
	struct HashNode* next;
} HashNode;

typedef struct {
	unsigned int count;
	unsigned int capacity;
	HashNode** arr;
} HashMap;


void put_node(HashMap* map, char* name, int name_len, unsigned int val);
void create_variable_map(HashMap* mp);
void free_hash_map(HashMap* map);
HashNode* get_node(HashMap* map, char* name, int name_len);

#endif // !SHIP_TABLE_H_
