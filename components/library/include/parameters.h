
// #ifndef __PARAMETERS_H__
// #define __PARAMETERS_H__

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// #include "nvs.h"
// #include "nvs_flash.h"

// typedef enum DataTypes_e {
//     type_char,
//     type_int,
//     type_uint
// } DataTypes_t;

// typedef struct Parameter_s {
//     DataTypes_t type;
//     uint size;  // Used for declaring arrays, max size of array
//     void* p;
//     char* name;
//     char* desc;
// } Parameter_t;

// typedef struct ParameterList_s {
// } ParameterList_t;

// extern ParameterList_t Params;

// #define PARAM_ARRAY(datatype, size, name, description)


// // void load_parameters();
// // void save_parameters();
// // void set_param1(int value);
// // void set_param2(float value);
// // void set_param3(const char *value);
// // void schedule_save();

// // #include <stdio.h>
// // #include <string.h>

// // #include "esp_timer.h"
// // #include "nvs.h"
// // #include "nvs_flash.h"

// // #define MAX_STR_LENGTH 64

// // typedef enum {
// //     PARAM_TYPE_INT,
// //     PARAM_TYPE_UINT,
// //     PARAM_TYPE_STRING
// // } param_type_t;

// // typedef struct {
// //     const char* key;
// //     param_type_t type;
// //     union {
// //         int int_default;
// //         unsigned int uint_default;
// //         char* str_default;
// //     } default_value;
// //     union {
// //         int int_val;
// //         unsigned int uint_val;
// //         char str_val[MAX_STR_LENGTH];
// //     } current_value;
// //     bool dirty;
// // } parameter_t;

// // typedef struct {
// //     parameter_t* parameters;
// //     size_t num_parameters;
// //     esp_timer_handle_t timer;
// // } param_manager_t;

// // static void save_dirty_parameters(param_manager_t* manager)
// // {
// //     nvs_handle_t handle;
// //     esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
// //     if (err != ESP_OK) return;

// //     for (size_t i = 0; i < manager->num_parameters; i++) {
// //         parameter_t* p = &manager->parameters[i];
// //         if (p->dirty) {
// //             switch (p->type) {
// //                 case PARAM_TYPE_INT:
// //                     nvs_set_i32(handle, p->key, p->current_value.int_val);
// //                     break;
// //                 case PARAM_TYPE_UINT:
// //                     nvs_set_u32(handle, p->key, p->current_value.uint_val);
// //                     break;
// //                 case PARAM_TYPE_STRING:
// //                     nvs_set_str(handle, p->key, p->current_value.str_val);
// //                     break;
// //             }
// //             p->dirty = false;
// //         }
// //     }
// //     nvs_commit(handle);
// //     nvs_close(handle);
// // }

// // static void timer_callback(void* arg)
// // {
// //     param_manager_t* manager = (param_manager_t*)arg;
// //     save_dirty_parameters(manager);
// // }

// // esp_err_t param_manager_init(param_manager_t* manager, parameter_t* params, size_t num_params)
// // {
// //     esp_err_t ret = nvs_flash_init();
// //     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
// //         ESP_ERROR_CHECK(nvs_flash_erase());
// //         ret = nvs_flash_init();
// //     }
// //     ESP_ERROR_CHECK(ret);

// //     manager->parameters = params;
// //     manager->num_parameters = num_params;

// //     nvs_handle_t handle;
// //     ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &handle));

// //     for (size_t i = 0; i < num_params; i++) {
// //         parameter_t* p = &params[i];
// //         size_t required_size = 0;
// //         bool exists = false;

// //         switch (p->type) {
// //             case PARAM_TYPE_INT: {
// //                 int32_t val;
// //                 if (nvs_get_i32(handle, p->key, &val) == ESP_OK) {
// //                     p->current_value.int_val = val;
// //                     exists = true;
// //                 }
// //                 break;
// //             }
// //             case PARAM_TYPE_UINT: {
// //                 uint32_t val;
// //                 if (nvs_get_u32(handle, p->key, &val) == ESP_OK) {
// //                     p->current_value.uint_val = val;
// //                     exists = true;
// //                 }
// //                 break;
// //             }
// //             case PARAM_TYPE_STRING: {
// //                 if (nvs_get_str(handle, p->key, NULL, &required_size) == ESP_OK) {
// //                     if (required_size <= MAX_STR_LENGTH) {
// //                         nvs_get_str(handle, p->key, p->current_value.str_val, &required_size);
// //                         exists = true;
// //                     }
// //                 }
// //                 break;
// //             }
// //         }

// //         if (!exists) {
// //             switch (p->type) {
// //                 case PARAM_TYPE_INT:
// //                     p->current_value.int_val = p->default_value.int_default;
// //                     break;
// //                 case PARAM_TYPE_UINT:
// //                     p->current_value.uint_val = p->default_value.uint_default;
// //                     break;
// //                 case PARAM_TYPE_STRING:
// //                     strncpy(p->current_value.str_val, p->default_value.str_default, MAX_STR_LENGTH);
// //                     p->current_value.str_val[MAX_STR_LENGTH - 1] = '\0';
// //                     break;
// //             }
// //             p->dirty = true;
// //         }
// //     }

// //     save_dirty_parameters(manager);
// //     nvs_close(handle);

// //     // Setup periodic save timer
// //     const esp_timer_create_args_t timer_args = {
// //         .callback = timer_callback,
// //         .arg = manager,
// //         .name = "param_save"};
// //     ESP_ERROR_CHECK(esp_timer_create(&timer_args, &manager->timer));
// //     ESP_ERROR_CHECK(esp_timer_start_periodic(manager->timer, 10000000));  // 10 seconds

// //     return ESP_OK;
// // }

// // void param_manager_set_int(param_manager_t* manager, const char* key, int value)
// // {
// //     for (size_t i = 0; i < manager->num_parameters; i++) {
// //         parameter_t* p = &manager->parameters[i];
// //         if (strcmp(p->key, key) == 0 && p->type == PARAM_TYPE_INT) {
// //             if (p->current_value.int_val != value) {
// //                 p->current_value.int_val = value;
// //                 p->dirty = true;
// //             }
// //             return;
// //         }
// //     }
// // }

// // // Similar functions for uint and string types...

// #endif  // __PARAMETERS_H__


// // parameter_t parameters[] = {
// //     {"brightness", PARAM_TYPE_INT, .default_value.int_default = 50},
// //     {"interval", PARAM_TYPE_UINT, .default_value.uint_default = 1000},
// //     {"ssid", PARAM_TYPE_STRING, .default_value.str_default = "default_ssid"}
// // };

// // int app_main() {
// //     param_manager_t manager;
// //     ESP_ERROR_CHECK(param_manager_init(&manager, parameters, sizeof(parameters)/sizeof(parameters[0])));
    
// //     // Example usage
// //     param_manager_set_int(&manager, "brightness", 75);

    
// //     return 0;
// // }



// // // Edit command for console:
// typedef enum {
//     PARAM_TYPE_INT32,
//     PARAM_TYPE_UINT32,
//     PARAM_TYPE_CHAR_ARRAY,
//     PARAM_TYPE_INT_ARRAY,
//     // ... add more types as needed
// } EParamType_t;

// typedef struct {
//     const char* name;
//     EParamType_t type;
//     void* value;       // pointer to the parameter value in g_params
//     size_t size;       // For arrays, size in elements; for strings, max length; for others, size of data type
//     bool* dirty_flag;  // pointer to the dirty flag
// } ParamDescriptor_t;

// ParamDescriptor_t g_param_descriptors[] = {
//     {"brightness", PARAM_TYPE_INT32, &g_params.brightness_.value, sizeof(g_params.brightness_.value), &g_params.brightness_.dirty},
//     {"interval", PARAM_TYPE_UINT32, &g_params.interval_.value, sizeof(g_params.interval_.value), &g_params.interval_.dirty},
//     {"ssid", PARAM_TYPE_CHAR_ARRAY, g_params.ssid_.value, g_params.ssid_.size, &g_params.ssid_.dirty},
//     {"password", PARAM_TYPE_CHAR_ARRAY, g_params.password_.value, g_params.password_.size, &g_params.password_.dirty},
//     {"myarray", PARAM_TYPE_INT_ARRAY, g_params.myarray_.value, g_params.myarray_.size, &g_params.myarray_.dirty},
// };

// /// @brief Edit a parameter by name
// /// @param name name of the parameter you want to edit
// /// @param new_value value of the parameter
// /// @return esp_err code
// esp_err_t Param_Edit(char* name, void* new_value);

// esp_err_t Param_Edit(const char* name, void* new_value)
// {
//     for (size_t i = 0; i < sizeof(g_param_descriptors) / sizeof(ParamDescriptor_t); i++) {
//         if (strcmp(name, g_param_descriptors[i].name) == 0) {
//             switch (g_param_descriptors[i].type) {
//                 case PARAM_TYPE_INT32:
//                     *((int32_t*)g_param_descriptors[i].value) = *((int32_t*)new_value);
//                     break;
//                 case PARAM_TYPE_UINT32:
//                     *((uint32_t*)g_param_descriptors[i].value) = *((uint32_t*)new_value);
//                     break;
//                 case PARAM_TYPE_CHAR_ARRAY:
//                     // Assuming new_value is a null-terminated string.
//                     strncpy((char*)g_param_descriptors[i].value, (char*)new_value, g_param_descriptors[i].size);
//                     // Ensure null termination.
//                     ((char*)g_param_descriptors[i].value)[g_param_descriptors[i].size - 1] = '\0';
//                     break;
//                 case PARAM_TYPE_INT_ARRAY:
//                     memcpy(g_param_descriptors[i].value, new_value, sizeof(int) * g_param_descriptors[i].size);
//                     break;
//                 default:
//                     return ESP_ERR_INVALID_ARG;
//             }
//             *(g_param_descriptors[i].dirty_flag) = true;
//             return ESP_OK;
//         }
//     }
//     return ESP_ERR_NOT_FOUND;
// }