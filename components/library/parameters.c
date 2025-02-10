// #include <stdio.h>
// #include <string.h>
// #include "nvs_flash.h"
// #include "nvs.h"
// #include "esp_log.h"
// #include "esp_timer.h"

// #define NAMESPACE "params"
// #define SAVE_DELAY_MS 5000 // Delay before saving after a change

// typedef struct
// {
//     int param1;
//     float param2;
//     char param3[32];
// } system_params_t;

// static system_params_t params;
// static bool params_dirty = false;
// static esp_timer_handle_t save_timer;

// // Function declarations
// void load_parameters();
// void save_parameters();
// void set_param1(int value);
// void set_param2(float value);
// void set_param3(const char *value);
// void schedule_save();

// void load_parameters()
// {
//     esp_err_t err = nvs_flash_init();
//     if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
//     {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         err = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(err);

//     nvs_handle_t nvs_handle;
//     err = nvs_open(NAMESPACE, NVS_READWRITE, &nvs_handle);
//     if (err != ESP_OK)
//     {
//         ESP_LOGE("NVS", "Error opening NVS!");
//         return;
//     }

//     // Read parameters
//     int32_t param1;
//     float param2;
//     size_t required_size = sizeof(params.param3);

//     if (nvs_get_i32(nvs_handle, "param1", &param1) == ESP_OK)
//         params.param1 = param1;
//     if (nvs_get_flt(nvs_handle, "param2", &param2) == ESP_OK)
//         params.param2 = param2;
//     if (nvs_get_str(nvs_handle, "param3", params.param3, &required_size) != ESP_OK)
//         strcpy(params.param3, "default");

//     nvs_close(nvs_handle);
//     ESP_LOGI("NVS", "Parameters loaded.");
// }

// void set_param1(int value)
// {
//     if (params.param1 != value)
//     {
//         params.param1 = value;
//         params_dirty = true;
//         schedule_save();
//     }
// }

// void set_param2(float value)
// {
//     if (params.param2 != value)
//     {
//         params.param2 = value;
//         params_dirty = true;
//         schedule_save();
//     }
// }

// void set_param3(const char *value)
// {
//     if (strcmp(params.param3, value) != 0)
//     {
//         strncpy(params.param3, value, sizeof(params.param3));
//         params_dirty = true;
//         schedule_save();
//     }
// }

// void save_parameters()
// {
//     if (!params_dirty)
//         return;

//     nvs_handle_t nvs_handle;
//     esp_err_t err = nvs_open(NAMESPACE, NVS_READWRITE, &nvs_handle);
//     if (err != ESP_OK)
//     {
//         ESP_LOGE("NVS", "Failed to open NVS for writing");
//         return;
//     }

//     // Write parameters
//     nvs_set_i32(nvs_handle, "param1", params.param1);
//     nvs_set_flt(nvs_handle, "param2", params.param2);
//     nvs_set_str(nvs_handle, "param3", params.param3);
//     nvs_commit(nvs_handle);
//     nvs_close(nvs_handle);

//     params_dirty = false;
//     ESP_LOGI("NVS", "Parameters saved.");
// }

// void save_timer_callback(void *arg)
// {
//     save_parameters();
// }

// void schedule_save()
// {
//     esp_timer_create_args_t timer_args = {
//         .callback = &save_timer_callback,
//         .name = "param_save_timer"};

//     if (save_timer == NULL)
//     {
//         esp_timer_create(&timer_args, &save_timer);
//     }

//     esp_timer_start_once(save_timer, SAVE_DELAY_MS * 1000);
// }

// ---------------------------------------------------------

// static int set_ssid_handler(int argc, char **argv) {
//     if (argc != 2) {
//         printf("Usage: %s <new_ssid>\n", argv[0]);
//         return 1;
//     }

//     const char *new_ssid = argv[1];
//     size_t ssid_len = strlen(new_ssid);
    
//     // Validate SSID length
//     if (ssid_len >= sizeof(Params.SSID)) {
//         printf("Error: SSID must be %d characters or less\n", 
//                sizeof(Params.SSID) - 1);
//         return 1;
//     }

//     // Save with existing password
//     if (save_wifi_credentials(new_ssid, Params.Password) != EXIT_SUCCESS) {
//         printf("Failed to save SSID to NVS\n");
//         return 1;
//     }

//     // Update in-memory parameters
//     strncpy(Params.SSID, new_ssid, sizeof(Params.SSID));
//     printf("SSID updated to: %s\n", Params.SSID);
//     return 0;
// }

// static int set_password_handler(int argc, char **argv) {
//     if (argc != 2) {
//         printf("Usage: %s <new_password>\n", argv[0]);
//         return 1;
//     }

//     const char *new_password = argv[1];
//     size_t pass_len = strlen(new_password);
    
//     // Validate password length
//     if (pass_len >= sizeof(Params.Password)) {
//         printf("Error: Password must be %d characters or less\n",
//                sizeof(Params.Password) - 1);
//         return 1;
//     }

//     // Save with existing SSID
//     if (save_wifi_credentials(Params.SSID, new_password) != EXIT_SUCCESS) {
//         printf("Failed to save password to NVS\n");
//         return 1;
//     }

//     // Update in-memory parameters
//     strncpy(Params.Password, new_password, sizeof(Params.Password));
//     printf("Password updated successfully\n");
//     return 0;
// }

// void register_wifi_commands(void) {
//     // Register SSID command
//     const esp_console_cmd_t ssid_cmd = {
//         .command = "set_ssid",
//         .help = "Set new WiFi SSID (max 31 chars)",
//         .hint = "<new_ssid>",
//         .func = &set_ssid_handler
//     };
//     ESP_ERROR_CHECK(esp_console_cmd_register(&ssid_cmd));

//     // Register password command
//     const esp_console_cmd_t pass_cmd = {
//         .command = "set_password",
//         .help = "Set new WiFi password (max 63 chars)",
//         .hint = "<new_password>",
//         .func = &set_password_handler
//     };
//     ESP_ERROR_CHECK(esp_console_cmd_register(&pass_cmd));
// }
