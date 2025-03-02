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
    PARAM(0, char, exampleChar, 'A', "example char", ExampleChar)                    \
    PARAM(0, uint8_t, exampleUint8, 255, "example uint8_t", ExampleUint8)            \
    PARAM(0, uint16_t, exampleUint16, 65535, "example uint16_t", ExampleUint16)      \
    PARAM(0, uint32_t, exampleUint32, 4294967295, "example uint32_t", ExampleUint32) \
    PARAM(0, int32_t, exampleInt32, -2147483648, "example int32_t", ExampleInt32)    \
    PARAM(0, float, exampleFloat, 3.14, "example float", ExampleFloat)               \
    PARAM(0, int32_t, brightness, 50, "brightness duh", Brightness)                  \
    PARAM(0, uint32_t, interval, 1000, "random interval", Internval)                 \
    PARAM(0, bool, seriousmode, false, "Determines AIs tone of voice", SeriousMode)  \
    ARRAY(0, char, 32, ssid, "fakessid", "WiFi ssid", Ssid)                          \
    ARRAY(0, char, 64, password, "fakepass", "WiFi password", Password)              \
    ARRAY(0, int32_t, 4, myarray, ARRAY_INIT(1, 0, 0, 0), "example int array", MyArray)

#define PARAM(secure_lvl_, type_, name_, default_value_, description_, pn) \
    struct {                                                               \
        const uint8_t secure_level;                                        \
        const char* name;                                                  \
        type_ value;                                                       \
        bool dirty;                                                        \
        const type_ default_value;                                         \
        const char* description;                                           \
        const char* const key;                                             \
    } name_;
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_, pn) \
    struct {                                                                      \
        const uint8_t secure_level;                                               \
        const char* name;                                                         \
        type_ value[size_];                                                       \
        const size_t size;                                                        \
        bool dirty;                                                               \
        const type_ default_value[size_];                                         \
        const char* description;                                                  \
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
    type_bool,
    type_char,
    type_uint8_t,
    type_uint16_t,
    type_uint32_t,
    type_int32_t,
    type_float,

    type_array_bool,
    type_array_char,
    type_array_uint8_t,
    type_array_uint16_t,
    type_array_uint32_t,
    type_array_int32_t,
    type_array_float,

    type_undefined,
};

typedef struct {
    const uint8_t secure_level;
    const char* name;
    enum EParamDataTypes type;
    void* value;       // pointer to the parameter value in g_params
    size_t size;       // For arrays, size in elements; for strings, max length; for others, size of data type
    bool* dirty_flag;  // pointer to the dirty flag
} ParamDescriptor_t;

esp_err_t Param_PrintScalar(const char* name, char* out_buffer);

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

#endif  // __PARAM_MANAGER_H__