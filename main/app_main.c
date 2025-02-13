/** app_main.c
 *
 *  Starting point into the project
 *
 */

#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_fat.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "cmd_system.h"
#include "cmd_nvs.h"
#include "../components/library/common.h"
#include "../components/library/parameters.h"
#include "wifi.h"
#include "internet_check.h"
#include "esp_log.h"

static const char *TAG = "app_main.c";

typedef enum DataTypes_e
{
    type_char,
    type_int,
    type_float,
} DataTypes_t;

int save_password(const char *password)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NETWORK_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return EXIT_FAILURE;
    }

    // Save password
    err = nvs_set_str(nvs_handle, "password", password);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to save Password!");
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to commit changes in NVS!");
    }

    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Wi-Fi password saved to NVS");
    return EXIT_SUCCESS;
}

int save_ssid(const char *ssid)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NETWORK_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return EXIT_FAILURE;
    }

    // Save SSID
    err = nvs_set_str(nvs_handle, "ssid", ssid);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to save SSID!");
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to commit changes in NVS!");
    }

    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Wi-Fi ssid saved to NVS");
    return EXIT_SUCCESS;
}

static int set_ssid_handler(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <new_ssid>\n", argv[0]);
        return 1;
    }

    const char *new_ssid = argv[1];
    size_t ssid_len = strlen(new_ssid);

    // Validate SSID length
    if (ssid_len >= 32)
    {
        printf("Error: SSID must be %d characters or less\n", 31);
        return 1;
    }

    // Save with existing password
    if (save_ssid(new_ssid) != EXIT_SUCCESS)
    {
        printf("Failed to save SSID to NVS\n");
        return 1;
    }

    // Update in-memory parameters
    // strncpy(Params.SSID, new_ssid, sizeof(Params.SSID));
    printf("SSID updated to: %s\n", new_ssid);
    return 0;
}

static int set_password_handler(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <new_password>\n", argv[0]);
        return 1;
    }

    const char *new_password = argv[1];
    size_t pass_len = strlen(new_password);

    // Save with existing SSID
    if (save_password(new_password) != EXIT_SUCCESS)
    {
        printf("Failed to save password to NVS\n");
        return 1;
    }

    // Update in-memory parameters
    // strncpy(Params.Password, new_password, sizeof(Params.Password));
    printf("Password updated successfully\n");
    return 0;
}

void register_wifi_commands(void)
{
    // Register SSID command
    const esp_console_cmd_t ssid_cmd = {
        .command = "set_ssid",
        .help = "Set new WiFi SSID (max 31 chars)",
        .hint = "<new_ssid>",
        .func = &set_ssid_handler};
    ESP_ERROR_CHECK(esp_console_cmd_register(&ssid_cmd));

    // Register password command
    const esp_console_cmd_t pass_cmd = {
        .command = "set_password",
        .help = "Set new WiFi password (max 63 chars)",
        .hint = "<new_password>",
        .func = &set_password_handler};
    ESP_ERROR_CHECK(esp_console_cmd_register(&pass_cmd));
}

static void initialize_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}


#define MOUNT_PATH "/data"
#define HISTORY_PATH MOUNT_PATH "/history.txt"

static void initialize_filesystem(void)
{
    static wl_handle_t wl_handle;
    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 4,
            .format_if_mount_failed = true
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount(MOUNT_PATH, "storage", &mount_config, &wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return;
    }
}


void app_main(void)
{
    printf("\n    PlantProtector \n\n");

    printf("    $$$$$$$\\ $$\\                   $$\\    $$$$$$$\\                    $$\\                        $$\\                        \n");
    printf("    $$  __$$\\$$ |                  $$ |   $$  __$$\\                   $$ |                       $$ |                       \n");
    printf("    $$ |  $$ $$ |$$$$$$\\ $$$$$$$\\$$$$$$\\  $$ |  $$ |$$$$$$\\  $$$$$$\\$$$$$$\\   $$$$$$\\  $$$$$$$\\$$$$$$\\   $$$$$$\\  $$$$$$\\   \n");
    printf("    $$$$$$$  $$ |\\____$$\\$$  __$$\\_$$  _| $$$$$$$  $$  __$$\\$$  __$$\\_$$  _| $$  __$$\\$$  _____\\_$$  _| $$  __$$\\$$  __$$\\  \n");
    printf("    $$  ____/$$ |$$$$$$$ $$ |  $$ |$$ |   $$  ____/$$ |  \\__$$ /  $$ |$$ |   $$$$$$$$ $$ /       $$ |   $$ /  $$ $$ |  \\__| \n");
    printf("    $$ |     $$ $$  __$$ $$ |  $$ |$$ |$$\\$$ |     $$ |     $$ |  $$ |$$ |$$\\$$   ____$$ |       $$ |$$\\$$ |  $$ $$ |       \n");
    printf("    $$ |     $$ \\$$$$$$$ $$ |  $$ |\\$$$$  $$ |     $$ |     \\$$$$$$  |\\$$$$  \\$$$$$$$\\ $$$$$$$\\  \\$$$$  \\$$$$$$  $$ |       \n");
    printf("    \\__|     \\__|\\_______\\__|  \\__| \\____/\\__|     \\__|      \\______/  \\____/ \\_______|\\_______|  \\____/ \\______/\\__|       \n");
    printf("\n\n");

    printf("    Now on \x1b[1mESP32-S3\x1b[0m! \n");
    printf("\x1b[90m    Copyright (c) 2025 Hossein Molavi | Protecting Plants Worldwide\x1b[0m\n\n");

    // Copyright line (dimmed)


    register_wifi_commands();

    // Initialize NVS
    initialize_nvs();
    
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    char ssid[32] = {0};
    char password[64] = {0};

    int res = load_wifi_credentials(ssid, sizeof(ssid), password, sizeof(password));
    if (res == EXIT_FAILURE)
    {
        printf("SAVING SSID AND PASSWORD TO NVS\n");
        strncpy(ssid, DEFAULT_SSID, sizeof(ssid));
        strncpy(password, DEFAULT_PASS, sizeof(password));
        save_wifi_credentials(ssid, password);
    }
    else
    {
        printf("LOADED SSID AND PASS from NVS!!\n");
    }

    wifi_init_sta(ssid, password);
    check_internet_connection();

    printf("---CMD SHOULD BE INTERACTIVE FROM HERE\n");
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    repl_config.prompt = "PlantProtector>";
    repl_config.max_cmdline_length = 150;

    initialize_nvs();

    // initialize_filesystem();
    // repl_config.history_save_path = HISTORY_PATH;
    // ESP_LOGI(TAG, "Command history enabled");

    /* Register commands */
    esp_console_register_help_command();
    register_system();
    register_nvs();

    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

    ESP_ERROR_CHECK(esp_console_start_repl(repl));

}