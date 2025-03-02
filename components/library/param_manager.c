///@file param_manager.c
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///
///@version 1.2
///@date 2025-03-02
///
///@copyright Copyright (c) 2025
///

#include "include/param_manager.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char* TAG = "param_manager.c";

#define NVS_NAMESPACE "param_storage"

///@brief Parameter storage initialization. Cant set the default
///       here because arrays need to be strncpy. Will be done in
///       Param_ManagerInit()
///       Stringify the name for the key with #name_
#define PARAM(type_, name_, default_value_, description_, pn) \
    .name_ = {                                                \
        .name = #name_,                                       \
        .dirty = false,                                       \
        .default_value = default_value_,                      \
        .description = description_,                          \
        .key = #name_,                                        \
    },
#define ARRAY(type_, size_, name_, default_value_, description_, pn) \
    .name_ = {                                                       \
        .name = #name_,                                              \
        .size = size_,                                               \
        .dirty = false,                                              \
        .default_value = default_value_,                             \
        .description = description_,                                 \
        .key = #name_,                                               \
    },
struct ParamMasterControl g_params = {
    PARAMETER_TABLE};
#undef PARAM
#undef ARRAY

///@brief Getters and Setters
///
#define PARAM(type_, name_, default_value_, description_, pn) \
    esp_err_t Param_Set##pn(const type_ value)                \
    {                                                         \
        if (g_params.name_.value != value) {                  \
            g_params.name_.value = value;                     \
            g_params.name_.dirty = true;                      \
            return ESP_OK;                                    \
        }                                                     \
        return ESP_FAIL;                                      \
    }                                                         \
    type_ Param_Get##pn(void)                                 \
    {                                                         \
        return g_params.name_.value;                          \
    }                                                         \
    esp_err_t Param_Reset##pn(void)                           \
    {                                                         \
        g_params.name_.value = default_value_;                \
        return ESP_OK;                                        \
    }
#define ARRAY(type_, size_, name_, default_value_, description_, pn)                         \
    esp_err_t Param_Set##pn(const type_* value, size_t length)                               \
    {                                                                                        \
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

/// @brief Identifies parameters that have been modified (dirty) and saves them to nvs
void Param_SaveDirtyParameters(void)
{
    nvs_handle_t handle;
    esp_err_t err;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return;
    int parametersChanged;
    parametersChanged = 0;

#define PARAM(type_, name_, default_value_, description_, pn)                                        \
    if (g_params.name_.dirty) {                                                                      \
        size_t name_##required_size = sizeof(type_);                                                 \
        err = nvs_set_blob(handle, g_params.name_.key, &g_params.name_.value, name_##required_size); \
        if (err != ESP_OK) ESP_LOGE(TAG, "Failed to set blob for: %s\n", g_params.name_.name);       \
        g_params.name_.dirty = false;                                                                \
        parametersChanged++;                                                                         \
    }
#define ARRAY(type_, size_, name_, default_value_, description_, pn)                                 \
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
    Param_SaveDirtyParameters();
}

/// @brief Attempt to pull g_params from nvs flash, if value failed or non-existant,
///        value will be set to default and dirty flag will be set to true. Also
///        creates periodic timer for nvs parameter saves; set to 30 seconds
void Param_ManagerInit(void)
{
    // NVS initialization
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    nvs_handle_t handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) == ESP_OK) {
#define PARAM(type_, name_, default_value_, description_, pn)                                                \
    size_t name_##_required_size = sizeof(g_params.name_.value);                                             \
    if (nvs_get_blob(handle, g_params.name_.key, &g_params.name_.value, &name_##_required_size) != ESP_OK) { \
        g_params.name_.value = g_params.name_.default_value;                                                 \
        g_params.name_.dirty = true;                                                                         \
    }                                                                                                        \
    else {                                                                                                   \
        g_params.name_.dirty = false;                                                                        \
    }
#define ARRAY(type_, size_, name_, default_value_, description_, pn)                                         \
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
    if (name == NULL) return type_undefined;

#define PARAM(t, name_, default_value_, description_, pn) \
    if (strncmp(#name_, name, strlen(#name_)) == 0) {     \
        return type_##t;                                  \
    }

#define ARRAY(type_, size_, name_, default_value_, description_, pn) \
    if (strncmp(#name_, name, strlen(#name_)) == 0) {                \
        return type_array_##type_;                                   \
    }
    PARAMETER_TABLE
#undef PARAM
#undef ARRAY

    /* The parameter does not exist */
    return type_undefined;
}