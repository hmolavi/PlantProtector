// param_manager.h
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

///@brief Program variable parameters. Format is
///       (type, variable name, default value, description),
///       For arrays, you must set the max length as well.
#define PARAMETER_TABLE                                \
    PARAM(int32_t, brightness, 50, "brightness duh")   \
    PARAM(uint32_t, interval, 1000, "random interval") \
    ARRAY(char, 64, ssid, "somessidval_def", "default_ssid")

#define PARAM(type_, name_, default_value_, description_) \
    struct {                                              \
        const char* name;                                 \
        type_ value;                                      \
        bool dirty;                                       \
        const type_ defaultValue;                         \
        const char* description;                          \
        const char* const key;                            \
    } name_;
#define ARRAY(type_, size_, name_, default_value_, description_) \
    struct {                                                     \
        const char* name;                                        \
        type_ value[size_];                                      \
        const uint32_t size;                                     \
        bool dirty;                                              \
        const type_ defaultValue[size_];                         \
        const char* description;                                 \
        const char* const key;                                   \
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
#define PARAM(type, name, default, description) \
    void set_##name(const type value);          \
    type get_##name(void);
#define ARRAY(type, size, name, default_value, description) \
    void set_##name(const type* value);                     \
    type* get_##name(void);
PARAMETER_TABLE
#undef PARAM
#undef ARRAY

void param_manager_init(void);