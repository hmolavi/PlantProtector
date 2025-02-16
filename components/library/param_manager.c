// param_manager.c
#include "include/param_manager.h"

#include <stdbool.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char* TAG = "param_manager.c";

///@brief Parameter storage initialization. Cant set the default
///       here because arrays need to be strncpy. Will be done in
///       param_manager_init()
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
#define PARAM(type, name, default, description, pn) \
    void Param_Set##pn(const type value)            \
    {                                               \
        g_params.name.value = value;                \
        g_params.name.dirty = true;                 \
    }                                               \
    type Param_Get##pn(void)                        \
    {                                               \
        return g_params.name.value;                 \
    }
#define ARRAY(type, size, name, default, description, pn)         \
    void Param_Set##pn(const type* value)                         \
    {                                                             \
        memcpy(&g_params.name.value, value, size * sizeof(type)); \
        g_params.name.dirty = true;                               \
    }                                                             \
    type* Param_Get##pn(void)                                     \
    {                                                             \
        return g_params.name.value;                               \
    }
PARAMETER_TABLE
#undef PARAM
#undef ARRAY

static void save_dirty_parameters(void)
{
    nvs_handle_t handle;
    esp_err_t err;
    if (nvs_open("storage", NVS_READWRITE, &handle) != ESP_OK) return;
    int parametersChanged;
    parametersChanged = 0;

#define PARAM(type_, name_, default_value_, description_, pn)                                        \
    if (g_params.name_.dirty) {                                                                      \
        size_t name_##required_size = sizeof(g_params.name_.value);                                  \
        err = nvs_set_blob(handle, g_params.name_.key, &g_params.name_.value, name_##required_size); \
        if (err != ESP_OK) {                                                                         \
            ESP_LOGE(TAG, "Failed to set blob for: %s\n", g_params.name_.name);                      \
        }                                                                                            \
        g_params.name_.dirty = false;                                                                \
        parametersChanged++;                                                                         \
    }
#define ARRAY(type_, size_, name_, default_value_, description_, pn)                                 \
    if (g_params.name_.dirty) {                                                                      \
        size_t name_##required_size = sizeof(g_params.name_.value);                                  \
        err = nvs_set_blob(handle, g_params.name_.key, &g_params.name_.value, name_##required_size); \
        if (err != ESP_OK) {                                                                         \
            ESP_LOGE(TAG, "Failed to set blob for: %s\n", g_params.name_.name);                      \
        }                                                                                            \
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

/// @brief Attempt to pull g_params from nvs flash, if value failed or non-existant,
///        value will be set to default and dirty flag will be set to true. Also
///        creates periodic timer for nvs parameter saves; set to 30 seconds
void param_manager_init(void)
{
    // NVS initialization
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READWRITE, &handle) == ESP_OK) {
#define PARAM(type_, name_, default_value_, description_, pn)                                               \
    size_t name_##required_size = sizeof(g_params.name_.value);                                             \
    if (nvs_get_blob(handle, g_params.name_.key, &g_params.name_.value, &name_##required_size) != ESP_OK) { \
        g_params.name_.value = g_params.name_.default_value;                                                \
        g_params.name_.dirty = true;                                                                        \
    }                                                                                                       \
    else {                                                                                                  \
        g_params.name_.dirty = false;                                                                       \
    }
#define ARRAY(type_, size_, name_, default_value_, description_, pn)                                        \
    size_t name_##required_size = sizeof(g_params.name_.value);                                             \
    if (nvs_get_blob(handle, g_params.name_.key, &g_params.name_.value, &name_##required_size) != ESP_OK) { \
        memcpy(&g_params.name_.value, &g_params.name_.default_value, size_ * sizeof(type_));                \
        g_params.name_.dirty = true;                                                                        \
    }                                                                                                       \
    else {                                                                                                  \
        g_params.name_.dirty = false;                                                                       \
    }

        PARAMETER_TABLE
#undef PARAM
#undef ARRAY

        nvs_close(handle);
    }

    // Periodic save timer
    static esp_timer_handle_t timer;
    const esp_timer_create_args_t timer_args = {
        .callback = (esp_timer_cb_t)save_dirty_parameters,
        .name = "g_param_save"};
    esp_timer_create(&timer_args, &timer);
    esp_timer_start_periodic(timer, 30 * 1000000);  // 30 seconds
}