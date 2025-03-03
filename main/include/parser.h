///@file parser.h
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief
///@version 1.0
///@date 2025-03-02
///
///@copyright Copyright (c) 2025
///

#ifndef __PARSER_H__
#define __PARSER_H__

#include "../../components/parameters/include/param_manager.h"

void strip_quotes(char* str);

bool parse_bool(const char* str, ParamValue_t* val);

bool parse_float(const char* str, ParamValue_t* val);

bool parse_uint32(const char* str, ParamValue_t* val);

#endif  // __PARSER_H__