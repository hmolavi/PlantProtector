///@file commands.c
///@brief Location for all of the command handlers
///@version 1.0
///@date 2025-02-13
///

// #include "include/commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../components/library/include/param_manager.h"
#include "../components/library/include/secure_level.h"
#include "esp_console.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "include/ascii_art.h"
#include "include/parser.h"
#include "include/wifi.h"

esp_err_t Cmd_Art(int argc, char **argv)
{
    PrintAsciiArt();
    return ESP_OK;
}

esp_err_t Cmd_Ssid(int argc, char **argv)
{
    if (argc != 2) {
        printf("\n\nUsage: %s <new_ssid>\n", argv[0]);
        return ESP_ERR_INVALID_ARG;
    }

    const char *new_ssid = argv[1];
    size_t new_ssid_len = strlen(new_ssid);

    size_t max_ssid_len;
    const char *cur_ssid = Param_GetSsid(&max_ssid_len);

    // Validate ssid length
    if (new_ssid_len >= max_ssid_len) {
        printf("Error: SSID must be %d characters or less\n", max_ssid_len);
        return ESP_ERR_INVALID_SIZE;
    }

    printf("\nssid: (%s) -> (%s) ...", cur_ssid, new_ssid);
    esp_err_t err = Param_SetSsid(new_ssid, new_ssid_len);
    if (err != ESP_OK) {
        printf("Error: Failed with error code (%s)\n", esp_err_to_name(err));
        return err;
    }
    else {
        printf("Done\n");
    }

    return ESP_OK;
}

esp_err_t Cmd_Password(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: %s <new_password>\n", argv[0]);
        return ESP_ERR_INVALID_ARG;
    }

    const char *new_password = argv[1];
    size_t new_password_len = strlen(new_password);

    size_t max_password_len;
    const char *cur_password = Param_GetPassword(&max_password_len);

    // Validate password length
    if (new_password_len >= max_password_len) {
        printf("Error: Password must be %d characters or less\n", max_password_len);
        return ESP_ERR_INVALID_SIZE;
    }

    printf("\npassword: (%s) -> (%s) ...", cur_password, new_password);
    esp_err_t err = Param_SetPassword(new_password, new_password_len);
    if (err != ESP_OK) {
        printf("Error: Failed with error code (%s)\n", esp_err_to_name(err));
        return err;
    }
    else {
        printf("Done\n");
    }

    return ESP_OK;
}

esp_err_t Cmd_Reboot(int argc, char **argv)
{
    esp_restart();
    // If board can't reboot means command failed
    return ESP_FAIL;
}

esp_err_t Cmd_Connect(int argc, char **argv)
{
    Wifi_TryConnect();
    return ESP_OK;
}

esp_err_t Cmd_Save(int argc, char **argv)
{
    printf("Saving dirty parameters\n");
    ParamManager_SaveDirtyParameters();
    return ESP_OK;
}

esp_err_t Cmd_Brightness(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: %s <brightness>\n", argv[0]);
        return ESP_ERR_INVALID_ARG;
    }

    int32_t brightness = atoi(argv[1]);
    if (brightness < -65535 || brightness > 65535) {
        printf("Error: Brightness must be between -65535 and 65535\n");
        return ESP_ERR_INVALID_ARG;
    }

    printf("\nbrightness: (%d) -> (%d) ...", Param_GetBrightness(), brightness);
    esp_err_t err = Param_SetBrightness(brightness);
    if (err != ESP_OK) {
        printf("Error: Failed with error code (%s)\n", esp_err_to_name(err));
        return err;
    }
    else {
        printf("Done\n");
    }

    return ESP_OK;
}

/*
esp_err_t Cmd_EditParam(int argc, char **argv)
{
    if (argc < 3) {
        printf("Usage: %s <parameter> <new_value>\n", argv[0]);
        return ESP_ERR_INVALID_ARG;
    }

    ParamDescriptor_t *desc = ParamManager_LookUp(argv[1]);
    if (!desc) {
        printf("Parameter '%s' not found\nEditable parameters:\n", argv[1]);
        ParamManager_PrintEditableParams();
        return ESP_ERR_NOT_FOUND;
    }

    ParamValue_t new_value = {0};
    esp_err_t ret = ESP_OK;

    // Handle different parameter types
    switch (desc->type) {
        case type_bool: {
            if (!parse_bool(argv[2], &new_value.b)) {
                printf("Invalid boolean value. Use true/false/1/0\n");
                return ESP_ERR_INVALID_ARG;
            }
            break;
        }

        case type_int32_t: {
            if (!parse_int32(argv[2], &new_value.i32)) {
                printf("Invalid integer format\n");
                return ESP_ERR_INVALID_ARG;
            }
            break;
        }

        case type_float: {
            if (!parse_float(argv[2], &new_value.f)) {
                printf("Invalid float format\n");
                return ESP_ERR_INVALID_ARG;
            }
            break;
        }

        case type_array_char: {
            // Combine all arguments after parameter name
            size_t total_len = 0;
            for (int i = 2; i < argc; i++) {
                total_len += strlen(argv[i]) + 1;  // Space for string + space
            }

            char *combined = malloc(total_len);
            combined[0] = '\0';

            for (int i = 2; i < argc; i++) {
                strcat(combined, argv[i]);
                if (i != argc - 1) strcat(combined, " ");
            }

            strip_quotes(combined);
            new_value.str = combined;

            if (strlen(new_value.str) >= desc->size) {
                printf("Value too long (max %zu chars)\n", desc->size - 1);
                free(combined);
                return ESP_ERR_INVALID_SIZE;
            }
            break;
        }

        default:
            printf("Unsupported parameter type\n");
            return ESP_ERR_NOT_SUPPORTED;
    }

    // Use descriptor's Set function
    if ((ret = desc->Set(new_value)) != ESP_OK) {
        printf("Failed to set parameter: %s\n", esp_err_to_name(ret));
        if (desc->type == type_array_char) free(new_value.str);
        return ret;
    }

    // Handle string cleanup
    if (desc->type == type_array_char) {
        free(new_value.str);
    }

    // Mark parameter as dirty and non-factory
    *desc->is_dirty = true;
    *desc->is_default = false;

    // Print confirmation
    char buf[128];
    desc->Print(buf, sizeof(buf));
    printf("Updated %s to: %s\n", desc->name, buf);

    return ESP_OK;
}
*/

// esp_err_t Cmd_EditParam(int argc, char **argv) {
//     if (argc == 1) {
//         ParamManager_PrintEditableParams();
//     }
//     if(argc < 3) {
//         printf("Usage: %s <parameter> <new_value>\n", argv[0]);
//         return ESP_ERR_INVALID_ARG;
//     }

//     ParamDescriptor_t* desc = ParamManager_LookUp(argv[1]);
//     if(!desc) {
//         printf("Parameter '%s' not found\n", argv[1]);
//         return ESP_ERR_NOT_FOUND;
//     }

//     ParamValue_t new_value = {0};
//     new_value.str = NULL; // Initialize string pointer

//     // Handle array types (special case for strings)
//     if(desc->type == type_array_char) {
//         // Combine all remaining arguments
//         size_t total_len = 0;
//         for(int i = 2; i < argc; i++) {
//             total_len += strlen(argv[i]) + 1;
//         }

//         char* combined = malloc(total_len);
//         combined[0] = '\0';

//         for(int i = 2; i < argc; i++) {
//             strcat(combined, argv[i]);
//             if(i != argc-1) strcat(combined, " ");
//         }

//         strip_quotes(combined);
//         new_value.str = combined;
//         new_value.size = strlen(combined);
//     }
//     else {
//         // Handle scalar values
//         if(!desc->parse_fn(argv[2], &new_value)) {
//             printf("Invalid value format for %s\n", desc->name);
//             return ESP_ERR_INVALID_ARG;
//         }
//     }

//     // Validate size limits
//     if(desc->type == type_array_char && new_value.size >= desc->size) {
//         printf("Value too long (max %zu)\n", desc->size-1);
//         free(new_value.str);
//         return ESP_ERR_INVALID_SIZE;
//     }

//     // Apply the new value
//     desc->set_fn((ParamValue_t*)desc->value, &new_value);
//     *desc->is_dirty = true;

//     // For strings, update is_default flag
//     if(desc->type == type_array_char) {
//         *desc->is_default = (strcmp(new_value.str, desc->default_value) == 0);
//         free(new_value.str);
//     }

//     printf("%s updated successfully\n", desc->name);
//     return ESP_OK;
// }
