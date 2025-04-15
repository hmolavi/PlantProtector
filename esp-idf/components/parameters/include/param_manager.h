///@file param_manager.h
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief Defined the program variable parameters used in the PlantProtector project.
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

#define PARAM(secure_lvl_, type_, name_, default_value_, description_) \
    struct {                                                           \
        const uint8_t secure_level;                                    \
        const char* name;                                              \
        type_ value;                                                   \
        const type_ default_value;                                     \
        bool is_dirty;                                                 \
        bool is_default;                                               \
        const char* const key;                                         \
    } name_;
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_) \
    struct {                                                                  \
        const uint8_t secure_level;                                           \
        const char* name;                                                     \
        type_ value[size_];                                                   \
        const size_t size;                                                    \
        const type_ default_value[size_];                                     \
        bool is_dirty;                                                        \
        bool is_default;                                                      \
        const char* const key;                                                \
    } name_;
typedef struct ParamMasterControl_s {
#include "param_table.inc"
} ParamMasterControl_t;
#undef PARAM
#undef ARRAY

/// @brief Program variables, stored on the SRAM. Initially variables
///        will be pulled from the NVS Flash, if they dont exist in
///        flash, they will be set to default value. Global variable
extern ParamMasterControl_t g_params;

// Getter and Setters for truly immutable access and no memory-tracking-hassle!
///@brief PARAM Getters, Setters, and Resetters are all trivial.
///       ARRAY Getters return a pointer to a constant of the appropriate type.
///       ARRAY Setters perform bound checks before setting the value.
///       ARRAY Resetters work as expected.
///       ARRAY Copy allows copying the array to a provided buffer.
#define PARAM(secure_lvl_, type_, name_, default_value_, description_) \
    esp_err_t Param_Set##name_(const type_ value);                     \
    type_ Param_Get##name_(void);                                      \
    esp_err_t Param_Reset##name_(void);
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_) \
    esp_err_t Param_Set##name_(const type_* value, size_t length);            \
    const type_* Param_Get##name_(size_t* out_length);                        \
    esp_err_t Param_Copy##name_(type_* buffer, size_t buffer_size);           \
    esp_err_t Param_Reset##name_(void);
#include "param_table.inc"
#undef PARAM
#undef ARRAY

/// @brief Param data types for all of PARAMs and ARRAYs
///
///        enum values are lowercase because they will be used
///        directly from the PARAM and ARRAY macros
typedef enum ParamDataTypes_e {
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
} ParamDataTypes_t;

/// @brief Union type for parameter values
///        Used to create another set of Setters/Getters/Print
///        functions for the ParamDescriptor_t
typedef union {
    // Scalar Types
    char c;
    bool b;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    int32_t i32;
    float f;

    // Array Types
    char* c_array;
    bool* b_array;
    uint8_t* u8_array;
    uint16_t* u16_array;
    uint32_t* u32_array;
    int32_t* i32_array;
    float* f_array;

} ParamValue_t;

typedef esp_err_t (*ParamGetFn)(ParamValue_t*);
typedef esp_err_t (*ParamSetFn)(ParamValue_t);
typedef esp_err_t (*ParamPrintFn)(char*, size_t);

/// @brief Descriptor for parameters, used to quickly find and
///        modify parameters from console
typedef struct {
    const uint8_t secure_level;
    const char* name;
    const ParamDataTypes_t type;
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

// uint32_t ParamManager_Get_uint32_t(ParamValue_t* val) { return val->u32; }
// void ParamManager_uint32_t_Set(ParamValue_t* val, ParamValue_t* ref) { val->u32 = ref->u32; }
// void Print_uint32_t(char* buf, ParamValue_t* val) { sprintf(buf, "%" PRIu32, val->u32); }

esp_err_t Param_Print(const char* name, char* out_buffer);

esp_err_t Param_PrintWithBufferSize(const char* name, char* out_buffer, const size_t buffer_size);

esp_err_t Param_PrintArray(const char* name, char** out_buffer, uint32_t* out_buffer_size);

/// @brief Attempts to pull g_params from nvs flash, if value failed or non-existant,
///        value will be set to default and dirty flag will be set to true. Also
///        creates periodic timer for nvs parameter saves; set to 30 seconds
esp_err_t ParamManager_Init(void);

/// @brief Identifies parameters that have been modified (dirty) and saves them to nvs
void ParamManager_SaveDirtyParameters(void);

///@brief Get the parameter data type
///
///@param name Name of the parameter
///@return enum ParamDataTypes_t
ParamDataTypes_t ParamManager_GetTypeByName(const char* name);

///@brief Find the parameter based on name
///
///@param name Name of the parameter
///@return ParamDescriptor_t*
ParamDescriptor_t* ParamManager_LookUp(const char* name);

/// @brief Print a list of editable parameters based on current secure level
void ParamManager_PrintEditableParams(void);

#endif  // __PARAM_MANAGER_H__