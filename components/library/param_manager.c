///@file param_manager.c
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///
///@version 1.2
///@date 2025-03-02
///
///@copyright Copyright (c) 2025
///

#include "include/param_manager.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "include/secure_level.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char* TAG = "param_manager.c";

#define NVS_NAMESPACE "param_storage"

///@brief Parameter storage initialization. Cant set the default
///       here because arrays need to be strncpy. Will be done in
///       ParamManager_Init()
///       Stringify the name for the key with #name_
#define PARAM(secure_lvl_, type_, name_, default_value_, description_, pn) \
    .name_ = {                                                             \
        .secure_level = secure_lvl_,                                       \
        .name = #name_,                                                    \
        .dirty = false,                                                    \
        .default_value = default_value_,                                   \
        .description = description_,                                       \
        .key = #name_,                                                     \
    },
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_, pn) \
    .name_ = {                                                                    \
        .secure_level = secure_lvl_,                                              \
        .name = #name_,                                                           \
        .size = size_,                                                            \
        .dirty = false,                                                           \
        .default_value = default_value_,                                          \
        .description = description_,                                              \
        .key = #name_,                                                            \
    },
struct ParamMasterControl g_params = {
    PARAMETER_TABLE};
#undef PARAM
#undef ARRAY

///@brief Getters and Setters
///
#define PARAM(secure_lvl_, type_, name_, default_value_, description_, pn) \
    esp_err_t Param_Set##pn(const type_ value)                             \
    {                                                                      \
        if (SecureLevel() > secure_lvl_) {                                 \
            return ESP_FAIL;                                               \
        }                                                                  \
        if (g_params.name_.value != value) {                               \
            g_params.name_.value = value;                                  \
            g_params.name_.dirty = true;                                   \
            return ESP_OK;                                                 \
        }                                                                  \
        return ESP_FAIL;                                                   \
    }                                                                      \
    type_ Param_Get##pn(void)                                              \
    {                                                                      \
        return g_params.name_.value;                                       \
    }                                                                      \
    esp_err_t Param_Reset##pn(void)                                        \
    {                                                                      \
        g_params.name_.value = default_value_;                             \
        return ESP_OK;                                                     \
    }
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_, pn)            \
    esp_err_t Param_Set##pn(const type_* value, size_t length)                               \
    {                                                                                        \
        if (SecureLevel() > secure_lvl_) {                                                   \
            return ESP_FAIL;                                                                 \
        }                                                                                    \
        if (length > size_) {                                                                \
            return ESP_ERR_INVALID_SIZE;                                                     \
        }                                                                                    \
        if (memcmp(&g_params.name_.value, value, size_ * sizeof(type_)) != 0) {              \
            memcpy(&g_params.name_.value, value, size_ * sizeof(type_));                     \
            g_params.name_.dirty = true;                                                     \
            return ESP_OK;                                                                   \
        }                                                                                    \
        return ESP_ERR_INVALID_ARG;                                                          \
    }                                                                                        \
    const type_* Param_Get##pn(size_t* out_length)                                           \
    {                                                                                        \
        if (out_length) *out_length = g_params.name_.size;                                   \
        return g_params.name_.value;                                                         \
    }                                                                                        \
    esp_err_t Param_Copy##pn(type_* buffer, size_t buffer_size)                              \
    {                                                                                        \
        const size_t required_size = g_params.name_.size * sizeof(type_);                    \
        if (buffer_size < required_size) {                                                   \
            return ESP_ERR_INVALID_SIZE;                                                     \
        }                                                                                    \
        memcpy(buffer, g_params.name_.value, required_size);                                 \
        return ESP_OK;                                                                       \
    }                                                                                        \
    esp_err_t Param_Reset##pn(void)                                                          \
    {                                                                                        \
        memcpy(&g_params.name_.value, &g_params.name_.default_value, size_ * sizeof(type_)); \
        return ESP_OK;                                                                       \
    }
PARAMETER_TABLE
#undef PARAM
#undef ARRAY

const ParamDescriptor_t ParamsDescriptor[] = {
#define PARAM(secure_lvl_, type__, name_, default_value_, description_, pn) \
    {                                                                       \
        .secure_level = secure_lvl_,                                        \
        .name = #name_,                                                     \
        .type = type_##type__,                                              \
        .value = &g_params.name_.value,                                     \
        .size = sizeof(type__),                                             \
        .dirty_flag = &g_params.name_.dirty,                                \
    },
#define ARRAY(secure_lvl_, type__, size_, name_, default_value_, description_, pn) \
    {                                                                              \
        .secure_level = secure_lvl_,                                               \
        .name = #name_,                                                            \
        .type = type_##array_##type__,                                             \
        .value = g_params.name_.value,                                             \
        .size = size_,                                                             \
        .dirty_flag = &g_params.name_.dirty,                                       \
    },
    PARAMETER_TABLE
#undef PARAM
#undef ARRAY
};
const uint32_t ParamsDescriptorSize = sizeof(ParamsDescriptor) / sizeof(ParamsDescriptor[0]);

esp_err_t Param_PrintScalar(const char* name, char* out_buffer)
{
    const size_t BUFFER_SIZE = 128;
    if (name == NULL || out_buffer == NULL) return ESP_ERR_INVALID_ARG;

    for (uint32_t i = 0; i < ParamsDescriptorSize; i++) {
        const ParamDescriptor_t* desc = &ParamsDescriptor[i];
        if (strcmp(desc->name, name) != 0) continue;

        // Reject array types
        if (desc->type >= type_array_bool) {
            return ESP_ERR_INVALID_ARG;
        }

        switch (desc->type) {
            case type_bool: {
                bool val = *(bool*)desc->value;
                snprintf(out_buffer, BUFFER_SIZE, "%s", val ? "true" : "false");
                return ESP_OK;
            }
            case type_char: {
                char c = *(char*)desc->value;
                snprintf(out_buffer, BUFFER_SIZE, "%c", c);
                return ESP_OK;
            }
            case type_uint8_t: {
                uint8_t val = *(uint8_t*)desc->value;
                snprintf(out_buffer, BUFFER_SIZE, "%" PRIu8, val);
                return ESP_OK;
            }
            case type_uint16_t: {
                uint16_t val = *(uint16_t*)desc->value;
                snprintf(out_buffer, BUFFER_SIZE, "%" PRIu16, val);
                return ESP_OK;
            }
            case type_uint32_t: {
                uint32_t val = *(uint32_t*)desc->value;
                snprintf(out_buffer, BUFFER_SIZE, "%" PRIu32, val);
                return ESP_OK;
            }
            case type_int32_t: {
                int32_t val = *(int32_t*)desc->value;
                snprintf(out_buffer, BUFFER_SIZE, "%" PRId32, val);
                return ESP_OK;
            }
            case type_float: {
                float val = *(float*)desc->value;
                snprintf(out_buffer, BUFFER_SIZE, "%.6g", val);
                return ESP_OK;
            }
            default:
                return ESP_ERR_NOT_SUPPORTED;
        }
    }
    return ESP_ERR_NOT_FOUND;
}

// Helper function to calculate array buffer size
static size_t calculate_array_buffer_size(const ParamDescriptor_t* desc)
{
    size_t max_element_size = 0;
    size_t num_elements = desc->size;

    switch (desc->type) {
        case type_array_bool:
            max_element_size = 5;  // "true" or "false"
            break;
        case type_array_char:
            return num_elements + 1;  // Simple string copy
        case type_array_uint8_t:
            max_element_size = 3;  // 0-255
            break;
        case type_array_uint16_t:
            max_element_size = 5;  // 0-65535
            break;
        case type_array_uint32_t:
            max_element_size = 10;  // 0-4294967295
            break;
        case type_array_int32_t:
            max_element_size = 11;  // -2147483648
            break;
        case type_array_float:
            max_element_size = 32;  // For scientific notation
            break;
        default:
            return 0;
    }

    // Calculate total size: (elements * (max_element_size + comma)) + null terminator
    return (num_elements * (max_element_size + 1)) + 1;
}

esp_err_t Param_PrintArray(const char* name, char** out_buffer, uint32_t* out_buffer_size)
{
    if (name == NULL || out_buffer == NULL) return ESP_ERR_INVALID_ARG;

    for (uint32_t i = 0; i < ParamsDescriptorSize; i++) {
        const ParamDescriptor_t* desc = &ParamsDescriptor[i];
        if (strcmp(desc->name, name) != 0) continue;

        if (desc->type < type_array_bool) {
            return ESP_ERR_INVALID_ARG;
        }

        // Calculate required buffer size
        size_t needed_size = calculate_array_buffer_size(desc);
        if (needed_size == 0) return ESP_ERR_NOT_SUPPORTED;

        // Allocate buffer
        *out_buffer = malloc(needed_size);
        if (*out_buffer == NULL) return ESP_ERR_NO_MEM;

        *out_buffer_size = needed_size;
        char* ptr = *out_buffer;
        *ptr = '\0';  // Initialize empty string

        switch (desc->type) {
            case type_array_char: {
                strncpy(ptr, (char*)desc->value, desc->size);
                ptr[desc->size] = '\0';
                return ESP_OK;
            }
            case type_array_bool: {
                bool* arr = (bool*)desc->value;
                for (size_t j = 0; j < desc->size; j++) {
                    const char* val_str = arr[j] ? "true" : "false";
                    if (j > 0) strcat(ptr, ",");
                    strcat(ptr, val_str);
                }
                return ESP_OK;
            }
            case type_array_uint8_t: {
                uint8_t* arr = (uint8_t*)desc->value;
                for (size_t j = 0; j < desc->size; j++) {
                    char num[4];
                    snprintf(num, sizeof(num), "%" PRIu8, arr[j]);
                    if (j > 0) strcat(ptr, ",");
                    strcat(ptr, num);
                }
                return ESP_OK;
            }
            case type_array_uint16_t: {
                uint16_t* arr = (uint16_t*)desc->value;
                for (size_t j = 0; j < desc->size; j++) {
                    char num[6];
                    snprintf(num, sizeof(num), "%" PRIu16, arr[j]);
                    if (j > 0) strcat(ptr, ",");
                    strcat(ptr, num);
                }
                return ESP_OK;
            }
            case type_array_uint32_t: {
                uint32_t* arr = (uint32_t*)desc->value;
                for (size_t j = 0; j < desc->size; j++) {
                    char num[11];
                    snprintf(num, sizeof(num), "%" PRIu32, arr[j]);
                    if (j > 0) strcat(ptr, ",");
                    strcat(ptr, num);
                }
                return ESP_OK;
            }
            case type_array_int32_t: {
                int32_t* arr = (int32_t*)desc->value;
                for (size_t j = 0; j < desc->size; j++) {
                    char num[12];
                    snprintf(num, sizeof(num), "%" PRId32, arr[j]);
                    if (j > 0) strcat(ptr, ",");
                    strcat(ptr, num);
                }
                return ESP_OK;
            }
            case type_array_float: {
                float* arr = (float*)desc->value;
                for (size_t j = 0; j < desc->size; j++) {
                    char num[32];
                    snprintf(num, sizeof(num), "%.6g", arr[j]);
                    if (j > 0) strcat(ptr, ",");
                    strcat(ptr, num);
                }
                return ESP_OK;
            }
            default:
                free(*out_buffer);
                *out_buffer = NULL;
                return ESP_ERR_NOT_SUPPORTED;
        }
    }
    return ESP_ERR_NOT_FOUND;
}

/// @brief Identifies parameters that have been modified (dirty) and saves them to nvs
void ParamManager_SaveDirtyParameters(void)
{
    nvs_handle_t handle;
    esp_err_t err;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return;
    int parametersChanged;
    parametersChanged = 0;

#define PARAM(secure_lvl_, type_, name_, default_value_, description_, pn)                           \
    if (g_params.name_.dirty) {                                                                      \
        size_t name_##required_size = sizeof(type_);                                                 \
        err = nvs_set_blob(handle, g_params.name_.key, &g_params.name_.value, name_##required_size); \
        if (err != ESP_OK) ESP_LOGE(TAG, "Failed to set blob for: %s\n", g_params.name_.name);       \
        g_params.name_.dirty = false;                                                                \
        parametersChanged++;                                                                         \
    }
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_, pn)                    \
    if (g_params.name_.dirty) {                                                                      \
        size_t name_##required_size = size_ * sizeof(type_);                                         \
        err = nvs_set_blob(handle, g_params.name_.key, &g_params.name_.value, name_##required_size); \
        if (err != ESP_OK) ESP_LOGE(TAG, "Failed to set blob for: %s\n", g_params.name_.name);       \
        g_params.name_.dirty = false;                                                                \
        parametersChanged++;                                                                         \
    }
    PARAMETER_TABLE
#undef PARAM
#undef ARRAY
    if (parametersChanged > 0) {
        printf("%d dirty parameters committing to flash...", parametersChanged);
        err = nvs_commit(handle);
        printf("%s\n", (err != ESP_OK) ? "Failed" : "Done");
    }
    nvs_close(handle);
}

static void save_dirty_parameters_callback(TimerHandle_t xTimer)
{
    ParamManager_SaveDirtyParameters();
}

/// @brief Attempt to pull g_params from nvs flash, if value failed or non-existant,
///        value will be set to default and dirty flag will be set to true. Also
///        creates periodic timer for nvs parameter saves; set to 30 seconds
void ParamManager_Init(void)
{
    // NVS initialization
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) == ESP_OK) {
#define PARAM(secure_lvl_, type_, name_, default_value_, description_, pn)                                   \
    size_t name_##_required_size = sizeof(g_params.name_.value);                                             \
    if (nvs_get_blob(handle, g_params.name_.key, &g_params.name_.value, &name_##_required_size) != ESP_OK) { \
        g_params.name_.value = g_params.name_.default_value;                                                 \
        g_params.name_.dirty = true;                                                                         \
    }                                                                                                        \
    else {                                                                                                   \
        g_params.name_.dirty = false;                                                                        \
    }
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_, pn)                            \
    size_t name_##_required_size = sizeof(g_params.name_.value);                                             \
    if (nvs_get_blob(handle, g_params.name_.key, &g_params.name_.value, &name_##_required_size) != ESP_OK) { \
        memcpy(&g_params.name_.value, &g_params.name_.default_value, sizeof(g_params.name_.value));          \
        g_params.name_.dirty = true;                                                                         \
    }                                                                                                        \
    else {                                                                                                   \
        g_params.name_.dirty = false;                                                                        \
    }

        PARAMETER_TABLE
#undef PARAM
#undef ARRAY

        nvs_close(handle);
    }

    // Periodic save timer
    TimerHandle_t xTimer = xTimerCreate(
        "g_param_save",
        pdMS_TO_TICKS(30000),  // Timer period in ticks (30 seconds)
        pdTRUE,
        (void*)0,
        save_dirty_parameters_callback);

    if (xTimer != NULL) {
        xTimerStart(xTimer, 0);
    }
    else {
        ESP_LOGE(TAG, "Failed to create periodic timer");
    }
}

enum EParamDataTypes ParamManager_GetTypeByName(const char* name)
{
    for (uint32_t i = 0; i < ParamsDescriptorSize; i++) {
        if (strcmp(ParamsDescriptor[i].name, name) == 0) {
            return ParamsDescriptor[i].type;
        }
    }
    return type_undefined;
}
