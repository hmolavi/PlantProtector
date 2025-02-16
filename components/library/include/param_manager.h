// param_manager.h
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

///@brief Program variable parameters. Format is
///       (type, variable name, default value, description,
///       PascalName). For arrays, you must set the max
///       length as well.
#define PARAMETER_TABLE                                           \
    PARAM(int32_t, brightness, 50, "brightness duh", Brightness)  \
    PARAM(uint32_t, interval, 1000, "random interval", Internval) \
    ARRAY(char, 64, ssid, "somessidval_def", "default_ssid", Ssid)

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
        const uint32_t size;                                         \
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

// Getter and Setters
#define PARAM(type, name, default, description, pn) \
    void Param_Set##pn(const type value);           \
    type Param_Get##pn(void);
#define ARRAY(type, size, name, default_value, description, pn) \
    void Param_Set##pn(const type* value);                      \
    type* Param_Get##pn(void);
PARAMETER_TABLE
#undef PARAM
#undef ARRAY

void param_manager_init(void);