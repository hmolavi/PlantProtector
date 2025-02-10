/** app_main.c
 *
 *  Starting point into the project
 *
 */

#include "../components/library/common.h"
#include "wifi.h"
#include "internet_check.h"
#include "esp_log.h"

static const char *TAG = "app_main.c";

void app_main(void)
{
    printf("PlantProtector \n\n");

    printf("    $$$$$$$\\ $$\\                   $$\\    $$$$$$$\\                    $$\\                        $$\\                        \n");
    printf("    $$  __$$\\$$ |                  $$ |   $$  __$$\\                   $$ |                       $$ |                       \n");
    printf("    $$ |  $$ $$ |$$$$$$\\ $$$$$$$\\$$$$$$\\  $$ |  $$ |$$$$$$\\  $$$$$$\\$$$$$$\\   $$$$$$\\  $$$$$$$\\$$$$$$\\   $$$$$$\\  $$$$$$\\   \n");
    printf("    $$$$$$$  $$ |\\____$$\\$$  __$$\\_$$  _| $$$$$$$  $$  __$$\\$$  __$$\\_$$  _| $$  __$$\\$$  _____\\_$$  _| $$  __$$\\$$  __$$\\  \n");
    printf("    $$  ____/$$ |$$$$$$$ $$ |  $$ |$$ |   $$  ____/$$ |  \\__$$ /  $$ |$$ |   $$$$$$$$ $$ /       $$ |   $$ /  $$ $$ |  \\__| \n");
    printf("    $$ |     $$ $$  __$$ $$ |  $$ |$$ |$$\\$$ |     $$ |     $$ |  $$ |$$ |$$\\$$   ____$$ |       $$ |$$\\$$ |  $$ $$ |       \n");
    printf("    $$ |     $$ \\$$$$$$$ $$ |  $$ |\\$$$$  $$ |     $$ |     \\$$$$$$  |\\$$$$  \\$$$$$$$\\ $$$$$$$\\  \\$$$$  \\$$$$$$  $$ |       \n");
    printf("    \\__|     \\__|\\_______\\__|  \\__| \\____/\\__|     \\__|      \\______/  \\____/ \\_______|\\_______|  \\____/ \\______/\\__|       \n");
    printf("\n\n");

    // Copyright line (dimmed)
    printf("\x1b[90mCopyright (c) 2025 Hossein Molavi | Protecting Plants Worldwide\x1b[0m\n");

    // Bolded subtitle
    printf("Now monitoring with \x1b[1mESP32-S3\x1b[0m! \n\n");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        printf("AYOOOOOOO\n");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

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
}
