#pragma once
#ifndef SHIP_COMPILER_H_
#define SHIP_COMPILER_H_

#include <stdbool.h>
#include "vm.h"


bool compile(const char* source, Chunk* chunk);

#endif 