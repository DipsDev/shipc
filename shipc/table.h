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

typedef struct {
    char* name;
    int length;
    Value val;
    struct ValueNode* next;
} ValueNode;

typedef struct {
    int count;
    int capacity;
    ValueNode ** arr;
} ValueTable;

void put_node(HashMap* map, char* name, int name_len, unsigned int val);
void create_variable_map(HashMap* mp);
void free_hash_map(HashMap* map);
HashNode* get_node(HashMap* map, char* name, int name_len);

// Table value related

void put_value_node(ValueTable * map,char* name,int name_len, Value val);
void create_value_map(ValueTable * mp);
ValueNode * get_global(ValueTable * map, char* name, int name_len);
void free_globals(ValueTable * map);

#endif // !SHIP_TABLE_H_
