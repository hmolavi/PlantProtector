///@file param_manager.h
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///
///@version 1.2
///@date 2025-03-02
///
///@copyright Copyright (c) 2025
///

#ifndef __PARAM_MANAGER_H__
#define __PARAM_MANAGER_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "esp_err.h"

#define ARRAY_INIT(...) {__VA_ARGS__}

///@brief Program variable parameters. Format is
///       (securelevel, type, variable name, default value, description,
///       PascalName). For arrays, you must set the max
///       length as well.
///
///       Available data types are:
///       char, uint8_t, uint16_t, uint32_t, int32_t, float
#define PARAMETER_TABLE                                                              \
    PARAM(2, char, exampleChar, 'A', "example char", ExampleChar)                    \
    PARAM(2, uint8_t, exampleUint8, 255, "example uint8_t", ExampleUint8)            \
    PARAM(2, uint16_t, exampleUint16, 65535, "example uint16_t", ExampleUint16)      \
    PARAM(2, uint32_t, exampleUint32, 4294967295, "example uint32_t", ExampleUint32) \
    PARAM(2, int32_t, exampleInt32, -2147483648, "example int32_t", ExampleInt32)    \
    PARAM(2, float, exampleFloat, 3.14, "example float", ExampleFloat)               \
    PARAM(2, int32_t, brightness, 50, "brightness duh", Brightness)                  \
    PARAM(2, uint32_t, interval, 1000, "random interval", Internval)                 \
    PARAM(2, bool, seriousmode, false, "Determines AIs tone of voice", SeriousMode)  \
    ARRAY(2, char, 32, ssid, "fakessid", "WiFi ssid", Ssid)                          \
    ARRAY(2, char, 64, password, "fakepass", "WiFi password", Password)              \
    ARRAY(2, int32_t, 4, myarray, ARRAY_INIT(1, 0, 0, 0), "example int array", MyArray)

#define PARAM(secure_lvl_, type_, name_, default_value_, description_, pn) \
    struct {                                                               \
        const uint8_t secure_level;                                        \
        const char* name;                                                  \
        type_ value;                                                       \
        const type_ default_value;                                         \
        bool is_dirty;                                                     \
        bool is_default;                                                   \
        const char* const key;                                             \
    } name_;
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_, pn) \
    struct {                                                                      \
        const uint8_t secure_level;                                               \
        const char* name;                                                         \
        type_ value[size_];                                                       \
        const size_t size;                                                        \
        const type_ default_value[size_];                                         \
        bool is_dirty;                                                            \
        bool is_default;                                                          \
        const char* const key;                                                    \
    } name_;
struct ParamMasterControl {
    PARAMETER_TABLE
};
#undef PARAM
#undef ARRAY

/// @brief Program variables, stored on the SRAM. Initially variables
///        will be pulled from the NVS Flash, if they dont exist in
///        flash, they will be set to default value. Global variable
extern struct ParamMasterControl g_params;

// Getter and Setters for truly immutable access and no memory-tracking-hassle!
///@brief PARAM Getters, Setters, and Resetters are all trivial.
///       ARRAY Getters return a pointer to a constant of the appropriate type.
///       ARRAY Setters perform bound checks before setting the value.
///       ARRAY Resetters work as expected.
///       ARRAY Copy allows copying the array to a provided buffer.
#define PARAM(secure_lvl_, type_, name_, default_value_, description_, pn) \
    esp_err_t Param_Set##pn(const type_ value);                            \
    type_ Param_Get##pn(void);                                             \
    esp_err_t Param_Reset##pn(void);
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_, pn) \
    esp_err_t Param_Set##pn(const type_* value, size_t length);                   \
    const type_* Param_Get##pn(size_t* out_length);                               \
    esp_err_t Param_Copy##pn(type_* buffer, size_t buffer_size);                  \
    esp_err_t Param_Reset##pn(void);
PARAMETER_TABLE
#undef PARAM
#undef ARRAY

/// @brief Param data types for all of PARAMs and ARRAYs
///
///        enum values are lowercase because they will be used
///        directly from the PARAM and ARRAY macros
enum EParamDataTypes {
    type_char,
    type_bool,
    type_uint8_t,
    type_uint16_t,
    type_uint32_t,
    type_int32_t,
    type_float,

    type_array_char,
    type_array_bool,
    type_array_uint8_t,
    type_array_uint16_t,
    type_array_uint32_t,
    type_array_int32_t,
    type_array_float,

    type_undefined,
};

/// @brief Union type for parameter values
///        Used to create another set of Setters/Getters/Print
///        functions for the ParamDescriptor_t
typedef union {
    bool b;
    char c;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    int32_t i32;
    float f;
    char* str;
    int32_t* i32_array;
} ParamValue_t;

typedef esp_err_t (*ParamGetFn)(ParamValue_t*);
typedef esp_err_t (*ParamSetFn)(ParamValue_t);
typedef esp_err_t (*ParamPrintFn)(char*, size_t);

/*
esp_err_t Param_Set_uint32_t(ParamValue_t *cur_val, ParamValue_t *new_val) {
    if (new_val.b != true && new_val.b != false) return ESP_ERR_INVALID_ARG;
    *(bool*)desc->value = new_val.b;
    return ESP_OK;
}

esp_err_t BoolParam_Set(ParamValue_t new_val) {
    if (new_val.b != true && new_val.b != false) return ESP_ERR_INVALID_ARG;
    *(bool*)desc->value = new_val.b;
    return ESP_OK;
}

esp_err_t BoolParam_Print(char* buffer, size_t buf_size) {
    snprintf(buffer, buf_size, "%s", *(bool*)desc->value ? "true" : "false");
    return ESP_OK;
}

// String array functions
esp_err_t StringParam_Set(ParamValue_t new_val) {
    strncpy((char*)desc->value, new_val.str, desc->size);
    ((char*)desc->value)[desc->size-1] = '\0'; // Ensure termination
    return ESP_OK;
}

esp_err_t StringParam_Print(char* buffer, size_t buf_size) {
    snprintf(buffer, buf_size, "\"%s\"", (char*)desc->value);
    return ESP_OK;
}
*/

//--------------------------------------------------
// Scalar type functions
//--------------------------------------------------

/*
#define GENERATE_SCALAR_FUNCTIONS(TYPE, FMT, CAST)              \
    esp_err_t TYPE##_Get(ParamValue_t* out)                     \
    {                                                           \
        out->CAST = *(typeof(out->CAST)*)desc->value;           \
        return ESP_OK;                                          \
    }                                                           \
    esp_err_t TYPE##_Set(ParamValue_t in)                       \
    {                                                           \
        *(typeof(in.CAST)*)desc->value = in.CAST;               \
        return ESP_OK;                                          \
    }                                                           \
    esp_err_t TYPE##_Print(char* buf, size_t sz)                \
    {                                                           \
        snprintf(buf, sz, FMT, *(typeof(in.CAST)*)desc->value); \
        return ESP_OK;                                          \
    }

GENERATE_SCALAR_FUNCTIONS(bool, "%s", b)
GENERATE_SCALAR_FUNCTIONS(char, "'%c'", c)
GENERATE_SCALAR_FUNCTIONS(uint8_t, "%" PRIu8, u8)
GENERATE_SCALAR_FUNCTIONS(uint16_t, "%" PRIu16, u16)
GENERATE_SCALAR_FUNCTIONS(uint32_t, "%" PRIu32, u32)
GENERATE_SCALAR_FUNCTIONS(int32_t, "%" PRId32, i32)
GENERATE_SCALAR_FUNCTIONS(float, "%.6g", f)

// Special bool handling
esp_err_t bool_Print(char* buf, size_t sz)
{
    snprintf(buf, sz, "%s", *(bool*)desc->value ? "true" : "false");
    return ESP_OK;
}

//--------------------------------------------------
// Array type functions
//--------------------------------------------------
esp_err_t array_char_Get(ParamValue_t* out)
{
    out->str = (char*)desc->value;
    return ESP_OK;
}
esp_err_t array_char_Set(ParamValue_t in)
{
    strncpy((char*)desc->value, in.str, desc->size);
    ((char*)desc->value)[desc->size - 1] = '\0';
    return ESP_OK;
}
esp_err_t array_char_Print(char* buf, size_t sz)
{
    snprintf(buf, sz, "\"%s\"", (char*)desc->value);
    return ESP_OK;
}

// int32 array functions
esp_err_t array_int32_t_Get(ParamValue_t* out)
{
    out->i32_array = (int32_t*)desc->value;
    return ESP_OK;
}
esp_err_t array_int32_t_Set(ParamValue_t in)
{
    memcpy(desc->value, in.i32_array, desc->size * sizeof(int32_t));
    return ESP_OK;
}
esp_err_t array_int32_t_Print(char* buf, size_t sz)
{
    char* ptr = buf;
    int32_t* arr = (int32_t*)desc->value;
    for (size_t i = 0; i < desc->size; i++) {
        ptr += snprintf(ptr, sz - (ptr - buf), "%s%d", (i > 0) ? "," : "", arr[i]);
        if (ptr - buf >= sz) break;
    }
    return ESP_OK;
}
*/

/// @brief Descriptor for parameters, used to quickly find and
///        modify parameters from console
typedef struct {
    const uint8_t secure_level;
    const char* name;
    const enum EParamDataTypes type;
    const char* description;
    void* value;       // pointer to the parameter value in g_params
    size_t size;       // For arrays, size in elements; for strings, max length; for others, size of data type
    bool* is_dirty;    // pointer to the dirty flag
    bool* is_default;  // pointer to the is_default flag

    // Function pointers
    ParamGetFn Get;
    ParamSetFn Set;
    ParamPrintFn Print;
} ParamDescriptor_t;



extern const ParamDescriptor_t g_params_descriptor[];
const uint32_t g_params_descriptor_size;

esp_err_t Param_Print(const char* name, char* out_buffer);

esp_err_t Param_PrintWithBufferSize(const char* name, char* out_buffer, const size_t buffer_size);

esp_err_t Param_PrintArray(const char* name, char** out_buffer, uint32_t* out_buffer_size);

/// @brief Attempts to pull g_params from nvs flash, if value failed or non-existant,
///        value will be set to default and dirty flag will be set to true. Also
///        creates periodic timer for nvs parameter saves; set to 30 seconds
void ParamManager_Init(void);

/// @brief Identifies parameters that have been modified (dirty) and saves them to nvs
void ParamManager_SaveDirtyParameters(void);

///@brief Get the parameter data type
///
///@param name Name of the parameter
///@return enum EParamDataTypes
enum EParamDataTypes ParamManager_GetTypeByName(const char* name);

///@brief Find the parameter based on name
///
///@param name Name of the parameter
///@return ParamDescriptor_t*
ParamDescriptor_t* ParamManager_LookUp(const char* name);

/// @brief Print a list of editable parameters based on current secure level
void ParamManager_PrintEditableParams(void);

#endif  // __PARAM_MANAGER_H__