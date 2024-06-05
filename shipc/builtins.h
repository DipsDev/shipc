#pragma once
#ifndef SHIP_BUILTINS_H
#define SHIP_BUILTINS_H

#include "objects.h"


// This module essentially provides the basic attributes for all primitive ship types. (strings, arrays, numbers, booleans etc)
Value get_builtin_attr(Value attr_host, StringObj* attr_given);

#endif //SHIP_BUILTINS_H
