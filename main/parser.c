///@file parser.c
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief
///@version 1.0
///@date 2025-03-02
///
///@copyright Copyright (c) 2025
///

#include "include/parser.h"

#include <stdbool.h>
#include <stdlib.h>

#include "../../components/parameters/include/param_manager.h"

void strip_quotes(char* str)
{
    size_t len = strlen(str);
    if (len >= 2 && str[0] == '"' && str[len - 1] == '"') {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }
}

// Type parsing functions
bool parse_bool(const char* str, ParamValue_t* val)
{
    if (strcasecmp(str, "true") == 0 || strcmp(str, "1") == 0) {
        val->b = true;
        return true;
    }
    if (strcasecmp(str, "false") == 0 || strcmp(str, "0") == 0) {
        val->b = false;
        return true;
    }
    return false;
}

bool parse_float(const char* str, ParamValue_t* val)
{
    char* endptr;
    val->f = strtof(str, &endptr);
    return *endptr == '\0';
}

bool parse_uint32(const char* str, ParamValue_t* val)
{
    char* endptr;
    val->u32 = strtoul(str, &endptr, 10);
    return *endptr == '\0';
}
