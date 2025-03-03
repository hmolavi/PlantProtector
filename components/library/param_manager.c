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

// #define DEBUG_INIT 1

#define DEFAULT_BUFFER_SIZE 128

///@brief Parameter storage initialization. Cant set the default
///       here because arrays need to be strncpy. Will be done in
///       ParamManager_Init()
///       Stringify the name for the key with #name_
#define PARAM(secure_lvl_, type_, name_, default_value_, description_, pn) \
    .name_ = {                                                             \
        .secure_level = secure_lvl_,                                       \
        .name = #name_,                                                    \
        .is_dirty = false,                                                 \
        .is_default = true,                                                \
        .default_value = default_value_,                                   \
        .key = #name_,                                                     \
    },
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_, pn) \
    .name_ = {                                                                    \
        .secure_level = secure_lvl_,                                              \
        .name = #name_,                                                           \
        .size = size_,                                                            \
        .is_dirty = false,                                                        \
        .is_default = true,                                                       \
        .default_value = default_value_,                                          \
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
            g_params.name_.is_default = false;                             \
            g_params.name_.is_dirty = true;                                \
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
            g_params.name_.is_default = false;                                               \
            g_params.name_.is_dirty = true;                                                  \
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

const ParamDescriptor_t g_params_descriptor[] = {
#define PARAM(secure_lvl_, type__, name_, ...)    \
    {                                             \
        .secure_level = secure_lvl_,              \
        .name = #name_,                           \
        .type = type_##type__,                    \
        .value = &g_params.name_.value,           \
        .size = sizeof(type__),                    \
        .is_dirty = &g_params.name_.is_dirty,     \
        .is_default = &g_params.name_.is_default, \
    },

#define ARRAY(secure_lvl_, type_, size_, name_, ...) \
    {                                                \
        .secure_level = secure_lvl_,                 \
        .name = #name_,                              \
        .type = type_array_##type_,                  \
        .value = g_params.name_.value,               \
        .size = size_,                               \
        .is_dirty = &g_params.name_.is_dirty,        \
        .is_default = &g_params.name_.is_default,    \
    },
    PARAMETER_TABLE
#undef PARAM
#undef ARRAY
};
const uint32_t g_params_descriptor_size = sizeof(g_params_descriptor) / sizeof(g_params_descriptor[0]);

esp_err_t Param_PrintWithBufferSize(const char* name, char* out_buffer, const size_t buffer_size)
{
    if (name == NULL || out_buffer == NULL) return ESP_ERR_INVALID_ARG;

    for (uint32_t i = 0; i < g_params_descriptor_size; i++) {
        const ParamDescriptor_t* desc = &g_params_descriptor[i];
        if (strcmp(desc->name, name) != 0) continue;

        // Reject array types with the exception of type_array_char
        if (desc->type >= type_array_bool) {
            return ESP_ERR_INVALID_ARG;
        }

        switch (desc->type) {
            case type_bool: {
                bool val = *(bool*)desc->value;
                snprintf(out_buffer, buffer_size, "%s", val ? "true" : "false");
                return ESP_OK;
            }
            case type_char: {
                char c = *(char*)desc->value;
                snprintf(out_buffer, buffer_size, "%c", c);
                return ESP_OK;
            }
            case type_uint8_t: {
                uint8_t val = *(uint8_t*)desc->value;
                snprintf(out_buffer, buffer_size, "%" PRIu8, val);
                return ESP_OK;
            }
            case type_uint16_t: {
                uint16_t val = *(uint16_t*)desc->value;
                snprintf(out_buffer, buffer_size, "%" PRIu16, val);
                return ESP_OK;
            }
            case type_uint32_t: {
                uint32_t val = *(uint32_t*)desc->value;
                snprintf(out_buffer, buffer_size, "%" PRIu32, val);
                return ESP_OK;
            }
            case type_int32_t: {
                int32_t val = *(int32_t*)desc->value;
                snprintf(out_buffer, buffer_size, "%" PRId32, val);
                return ESP_OK;
            }
            case type_float: {
                float val = *(float*)desc->value;
                snprintf(out_buffer, buffer_size, "%.6g", val);
                return ESP_OK;
            }
            case type_array_char: {
                char* str = (char*)desc->value;
                snprintf(out_buffer, buffer_size, "%s", str);
                return ESP_OK;
            }
            default:
                return ESP_ERR_NOT_SUPPORTED;
        }
    }
    return ESP_ERR_NOT_FOUND;
}

esp_err_t Param_Print(const char* name, char* out_buffer)
{
    return Param_PrintWithBufferSize(name, out_buffer, DEFAULT_BUFFER_SIZE);
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

    for (uint32_t i = 0; i < g_params_descriptor_size; i++) {
        const ParamDescriptor_t* desc = &g_params_descriptor[i];
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
    if (g_params.name_.is_dirty) {                                                                   \
        size_t name_##required_size = sizeof(type_);                                                 \
        err = nvs_set_blob(handle, g_params.name_.key, &g_params.name_.value, name_##required_size); \
        if (err != ESP_OK) ESP_LOGE(TAG, "Failed to set blob for: %s\n", g_params.name_.name);       \
        g_params.name_.is_dirty = false;                                                             \
        parametersChanged++;                                                                         \
    }
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_, pn)                    \
    if (g_params.name_.is_dirty) {                                                                   \
        size_t name_##required_size = size_ * sizeof(type_);                                         \
        err = nvs_set_blob(handle, g_params.name_.key, &g_params.name_.value, name_##required_size); \
        if (err != ESP_OK) ESP_LOGE(TAG, "Failed to set blob for: %s\n", g_params.name_.name);       \
        g_params.name_.is_dirty = false;                                                             \
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
        g_params.name_.is_default = true;                                                                    \
        g_params.name_.is_dirty = true;                                                                      \
    }                                                                                                        \
    else {                                                                                                   \
        g_params.name_.is_dirty = false;                                                                     \
        if (g_params.name_.value != g_params.name_.default_value) {                                          \
            g_params.name_.is_default = false;                                                               \
        }                                                                                                    \
        else {                                                                                               \
            g_params.name_.is_default = true;                                                                \
        }                                                                                                    \
    }
#define ARRAY(secure_lvl_, type_, size_, name_, default_value_, description_, pn)                            \
    size_t name_##_required_size = sizeof(g_params.name_.value);                                             \
    if (nvs_get_blob(handle, g_params.name_.key, &g_params.name_.value, &name_##_required_size) != ESP_OK) { \
        memcpy(&g_params.name_.value, &g_params.name_.default_value, sizeof(g_params.name_.value));          \
        g_params.name_.is_dirty = true;                                                                      \
        g_params.name_.is_default = true;                                                                    \
    }                                                                                                        \
    else {                                                                                                   \
        g_params.name_.is_dirty = false;                                                                     \
        if (memcmp(&g_params.name_.value, &g_params.name_.default_value, size_ * sizeof(type_)) != 0) {      \
            g_params.name_.is_default = false;                                                               \
        }                                                                                                    \
        else {                                                                                               \
            g_params.name_.is_default = true;                                                                \
        }                                                                                                    \
    }

        PARAMETER_TABLE
#undef PARAM
#undef ARRAY

        nvs_close(handle);
    }

#ifdef DEBUG_INIT
    const ParamDescriptor_t* p;
    p = g_params_descriptor;
    printf("\nParameters pulled from nvs:\n");
    for (uint32_t i = 0; i < g_params_descriptor_size; i++) {
        printf("%24s %s%s\n", p->name, *(p->is_default) ? "Factory" : "NOT Factory", *(p->is_dirty) ? " | dirty" : " ");
        p++;
    }
    printf("\n");
#endif  // DEBUG_INIT

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

ParamDescriptor_t* ParamManager_LookUp(const char* name)
{
    const ParamDescriptor_t* p = g_params_descriptor;
    for (uint32_t i = 0; i < g_params_descriptor_size; i++) {
        /*
            Since this function will be used mainly for user
            inputs, we can be lenient with case sensitivity
        */
        if (strcasecmp(name, p->name) == 0) {
            return p;
        }
        p++;
    }
    return NULL;
}

enum EParamDataTypes ParamManager_GetTypeByName(const char* name)
{
    for (uint32_t i = 0; i < g_params_descriptor_size; i++) {
        if (strcmp(g_params_descriptor[i].name, name) == 0) {
            return g_params_descriptor[i].type;
        }
    }
    return type_undefined;
}

void ParamManager_PrintEditableParams(void)
{
    const ParamDescriptor_t* p;
    char buf[DEFAULT_BUFFER_SIZE];
    p = g_params_descriptor;

    for (uint32_t i = 0; i < g_params_descriptor_size; i++) {
        if (p->secure_level >= SecureLevel()) {
            /* Make sure to dereference the is_dirty and is_default pointers */
            printf("%24s (%c%c)", p->name, *(p->is_dirty) ? '*' : ' ', *(p->is_default) ? 'F' : ' ');

            if (Param_Print(p->name, buf) == ESP_OK) {
                printf(" = %s\n", buf);
            }
            else {
                printf("\n");
            }
        }
        p++;
    }
}