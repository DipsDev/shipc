#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


// Helper macro to set the min and max arguments a builtin takes
#define REQ_ARGS(min, got, max) \
    if (got < min) return VAR_OBJ(create_err_obj("Not Enough Parameters", 21, ERR_SYNTAX)); \
    if (got > max) return VAR_OBJ(create_err_obj("Too much arguments given", 24, ERR_SYNTAX)) \

static Value validate_attr(StringObj* attr_obj, const char* real_attr, int real_attr_length, NativeFn fn) {
    if (attr_obj->length == real_attr_length && memcmp(attr_obj->value, real_attr, real_attr_length) == 0) {
        return VAR_OBJ(create_native_method_obj(fn));
    }
    return VAR_OBJ(create_err_obj("Object has no attribute", 23, ERR_NAME));
}

#define RUN_ATTR(attr_name, attr_length, func) validate_attr(attr_given, attr_name, attr_length, func)


// args[0] will always be the value the builtin is called on it
// For example: 4.to_str(14) -> Value[4, 14]

/*----------------------
 |  Number Builtins
 -----------------------*/

static Value to_str(int arg_count, Value* args) {
    REQ_ARGS(1, arg_count, 1);
    char string[32];
    snprintf(string, sizeof(string), "%g", AS_NUMBER(*args));
    StringObj* str_num = create_string_obj(string, strlen(string));
    return VAR_OBJ(str_num);
}

static Value num_attrs(StringObj* attr_given) {
    switch(attr_given->value[0]) {
        case 't': return RUN_ATTR("to_str", 6, to_str);
        default: return VAR_OBJ(create_err_obj("Number has no attribute", 23, ERR_NAME));
    }

}

Value get_builtin_attr(Value attr_host, StringObj* attr_given) {
    switch (attr_host.type) {
        case VAL_NUMBER: return num_attrs(attr_given);
        default: {
            printf("not implemented; builtins.c");
            return VAR_OBJ(create_err_obj("Not implemented; builtins.c", strlen("Not implemented; builtins.c"), ERR_TYPE));
        }
    }
}

#undef RUN_ATTR
#undef REQ_ARGS

