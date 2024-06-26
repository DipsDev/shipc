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
#define ATTRIBUTE_HOST(args) *args
#define ATTRIBUTE_ARGS(args) (args + 2);

#define ERROR(str, type) return VAR_OBJ(create_err_obj(str, strlen(str), type));


// args[0] will always be the value the builtin is called on it
// For example: 4.to_str(14) -> Value[4, 14]

/*----------------------
 |  Number Builtins
 -----------------------*/

/*
 * The function returns the string version of a number.
*/
static Value Number_to_str(int arg_count, Value* args) {
    REQ_ARGS(0, arg_count, 0);
    Value num = ATTRIBUTE_HOST(args);
    char string[50];
    snprintf(string, sizeof(string), "%g", AS_NUMBER(num));
    StringObj* str_num = create_string_obj(string, strlen(string));
    return VAR_OBJ(str_num);
}

/*
 * Returns: bool: whether the number is odd
 */
static Value Number_odd(int arg_count, Value* args) {
    REQ_ARGS(0, arg_count, 0);
    Value num = ATTRIBUTE_HOST(args);
    double d = 2.0;
    return VAR_BOOL(modf(AS_NUMBER(num), &d) != 0);
}

static Value Number_even(int arg_count, Value* args) {
    REQ_ARGS(0, arg_count, 0);
    Value num = ATTRIBUTE_HOST(args);
    double d = 2.0;
    return VAR_BOOL(modf(AS_NUMBER(num), &d) == 0);
}

/*
 * Returns the next consecutive integer.
 */
static Value Number_next(int arg_count, Value* args) {
    REQ_ARGS(0, arg_count, 0);
    Value num = ATTRIBUTE_HOST(args);
    return VAR_NUMBER(AS_NUMBER(num) + 1);
}

/*
 * Returns the previous consecutive integer.
 */
static Value Number_pred(int arg_count, Value* args) {
    REQ_ARGS(0, arg_count, 0);
    Value num = ATTRIBUTE_HOST( args);
    return VAR_NUMBER(AS_NUMBER(num) - 1);
}

/*
 * Returns an array that contains all numbers from 0 to n - 1;
 */
static Value Number_times(int arg_count, Value* args) {
    REQ_ARGS(0, arg_count, 0);
    ArrayObj* values = create_array_obj();
    Value num = ATTRIBUTE_HOST(args);
    for (int i = 0; i < AS_NUMBER(num); i++) {
        write_value_array(values->values, VAR_NUMBER(i));
    }
    return VAR_OBJ(values);
}

/*
 * Returns an array that contains all numbers fron i upto n including.
 */
static Value Number_upto(int arg_count, Value* args) {
    REQ_ARGS(1, arg_count, 1);
    Value bottom = ATTRIBUTE_HOST(args);
    Value top = *ATTRIBUTE_ARGS(args);

    if (!IS_NUMBER(top)) {
        ERROR("'Upto' expected number variable.", ERR_TYPE);
    }

    ArrayObj* arr = create_array_obj();

    for (int i = AS_NUMBER(bottom); i <= AS_NUMBER(top); i++) {
        write_value_array(arr->values, VAR_NUMBER(i));
    }
    return VAR_OBJ(arr);
}

static Value num_attrs(StringObj* attr_given) {
    switch(attr_given->value[0]) {
        case 't': {
            if (attr_given->length == 1) {
                return VAR_OBJ(create_err_obj("Number has no attribute", 23, ERR_NAME));
            }
            switch(attr_given->value[1]) {
                case 'o':
                    return RUN_ATTR("to_str", 6, Number_to_str);
                case 'i': return RUN_ATTR("times", 5, Number_times);
            }
        }
        case 'p': return RUN_ATTR("pred", 4, Number_pred);
        case 'e': return RUN_ATTR("even", 4, Number_even);
        case 'u': return RUN_ATTR("upto", 4, Number_upto);
        case 'o': return RUN_ATTR("odd", 3, Number_odd);
        case 'n': return RUN_ATTR("next", 4, Number_next);
        default: return VAR_OBJ(create_err_obj("Number has no attribute", 23, ERR_NAME));
    }

}


/*----------------------
 |  String Builtins
 -----------------------*/

/*
 * Returns the length of a string
 */
static Value String_length(int arg_count, Value* args) {
    REQ_ARGS(0, arg_count, 0);
    StringObj* str = AS_STRING(*args);
    return VAR_NUMBER(str->length);
}

static Value String_copy(int arg_count, Value* args) {
    REQ_ARGS(0, arg_count, 0);
    StringObj* str = AS_STRING(*args);
    StringObj* copy = create_string_obj(str->value, str->length);
    return VAR_OBJ(copy);
}

static Value String_capitalize(int arg_count, Value* args) {
    REQ_ARGS(0, arg_count, 0);
    StringObj* original = AS_STRING(*args);
    StringObj* str = create_string_obj(original->value, original->length);
    *str->value = *str->value & 0xDF;
    return VAR_OBJ(str);
}

static Value string_attrs(StringObj* attr_given) {
    switch(attr_given->value[0]) {
        case 'l': return RUN_ATTR("len", 3, String_length);
        case 't': return RUN_ATTR("title", 5, String_capitalize);
        case 'c': return RUN_ATTR("copy", 4, String_copy);
        default:
            ERROR("String has no attribute", ERR_NAME);
    }
}

/*----------------------
 |  String Builtins
 -----------------------*/

static Value Array_push(int arg_count, Value* args) {
    REQ_ARGS(1, arg_count, 1);
    ArrayObj* arr = AS_ARRAY(*args);
    Value val = *ATTRIBUTE_ARGS(args);
    write_value_array(arr->values, val);
    return VAR_NIL;
}

static Value Array_pop(int arg_count, Value* args) {
    REQ_ARGS(1, arg_count, 1);
    ArrayObj* arr = AS_ARRAY(*args);
    Value val = *ATTRIBUTE_ARGS(args);
    if (!IS_NUMBER(val)) {
        ERROR("pop(num) expected a number", ERR_TYPE);
    }
    double c = AS_NUMBER(val);
    if (c  == 0) {
        arr->values->count -= 1;
        return arr->values->arr[arr->values->count];
    }

    ArrayObj* new = create_array_obj();
    for (int i = arr->values->count - c - 1; i < arr->values->count; i++) {
        write_value_array(new->values, arr->values->arr[i]);
    }
    arr->values->count -= c + 1;

    return VAR_OBJ(new);
}

static Value Array_length(int arg_count, Value* args) {
    REQ_ARGS(0, arg_count, 0);
    ArrayObj* arr = AS_ARRAY(*args);
    return VAR_NUMBER(arr->values->count);
}

static Value array_attrs(StringObj* attr_given) {
    switch(attr_given->value[0]) {
        case 'l': return RUN_ATTR("len", 3, Array_length);
        case 'p': {
            if (attr_given->length == 1) {
                ERROR("Number has no attribute", ERR_NAME);
            }
            switch(attr_given->value[1]) {
                case 'o':
                    return RUN_ATTR("pop", 3, Array_pop);
                case 'u': return RUN_ATTR("push", 4, Array_push);
            }
        }
        default:
            ERROR("Array has no attribute", ERR_NAME);
    }
}

Value get_builtin_attr(Value attr_host, StringObj* attr_given) {
    switch (attr_host.type) {
        case VAL_NUMBER: return num_attrs(attr_given);
        case VAL_OBJ: {
            switch(AS_OBJ(attr_host)->type) {
                case OBJ_STRING: return string_attrs(attr_given);
                case OBJ_ARRAY: return array_attrs(attr_given);
                default:
                    ERROR("Not implemented; builtins.c", ERR_NAME);
            }
        }
        default: {
            printf("not implemented; builtins.c");
            ERROR("Not implemented; builtins.c", ERR_NAME);
        }
    }
}

#undef RUN_ATTR
#undef ATTRIBUTE_ARGS
#undef ATTRIBUTE_HOST
#undef REQ_ARGS
#undef ERROR

