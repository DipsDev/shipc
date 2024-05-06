#pragma once
#ifndef SHIP_TABLE_H
#define SHIP_TABLE_H

#include "value.h"

typedef struct {
	char* name;
	Value value;
	struct HashNode* next;
} HashNode;

typedef struct {
	unsigned int count;
	unsigned int capacity;
	HashNode** arr;
} HashMap;


void put_node(HashMap* map, char* name, int name_len, Value val);
void create_variable_map(HashMap* mp);
void free_hash_map(HashMap* map);
HashNode* get_node(HashMap* map, char* name, int name_len);

#endif // !SHIP_TABLE_H
