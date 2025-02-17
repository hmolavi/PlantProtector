// param_manager.h
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "esp_err.h"

#define ARRAY_INIT(...) {__VA_ARGS__}

///@brief Program variable parameters. Format is
///       (type, variable name, default value, description,
///       PascalName). For arrays, you must set the max
///       length as well.
#define PARAMETER_TABLE                                           \
    PARAM(int32_t, brightness, 50, "brightness duh", Brightness)  \
    PARAM(uint32_t, interval, 1000, "random interval", Internval) \
    ARRAY(char, 32, ssid, "", "WiFi ssid", Ssid)                  \
    ARRAY(char, 64, password, "", "WiFi password", Password)      \
    ARRAY(int, 4, myarray, ARRAY_INIT(1, 0, 0, 0), "example int array", MyArray)

#define PARAM(type_, name_, default_value_, description_, pn) \
    struct {                                                  \
        const char* name;                                     \
        type_ value;                                          \
        bool dirty;                                           \
        const type_ default_value;                            \
        const char* description;                              \
        const char* const key;                                \
    } name_;
#define ARRAY(type_, size_, name_, default_value_, description_, pn) \
    struct {                                                         \
        const char* name;                                            \
        type_ value[size_];                                          \
        const size_t size;                                           \
        bool dirty;                                                  \
        const type_ default_value[size_];                            \
        const char* description;                                     \
        const char* const key;                                       \
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
#define PARAM(type_, name_, default_value_, description_, pn) \
    esp_err_t Param_Set##pn(const type_ value);               \
    type_ Param_Get##pn(void);                                \
    esp_err_t Param_Reset##pn(void);
#define ARRAY(type_, size_, name_, default_value_, description_, pn) \
    esp_err_t Param_Set##pn(const type_* value, size_t length);      \
    const type_* Param_Get##pn(size_t* out_length);                  \
    esp_err_t Param_Copy##pn(type_* buffer, size_t buffer_size);     \
    esp_err_t Param_Reset##pn(void);
PARAMETER_TABLE
#undef PARAM
#undef ARRAY

/// @brief Attempts to pull g_params from nvs flash, if value failed or non-existant,
///        value will be set to default and dirty flag will be set to true. Also
///        creates periodic timer for nvs parameter saves; set to 30 seconds
void Param_ManagerInit(void);

/// @brief Identifies parameters that have been modified (dirty) and saves them to nvs
void Param_SaveDirtyParameters(void);