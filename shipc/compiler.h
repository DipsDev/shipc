#pragma once
#ifndef SHIP_COMPILER_H_
#define SHIP_COMPILER_H_

#include <stdbool.h>
#include "vm.h"
#include "objects.h"


FunctionObj* compile(const char* source);

#endif 