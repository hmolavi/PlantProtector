/** wifi.c
 *
 */

#include "../components/library/common.h"
#include "wifi.h"
#include "esp_log.h"
#include "esp_console.h"


#define EXAMPLE_ESP_MAXIMUM_RETRY 10

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG = "wifi.c";

static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

int save_wifi_credentials(const char *ssid, const char *password)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NETWORK_STORAGE_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return EXIT_FAILURE;
    }

    // Save SSID and password
    err = nvs_set_str(nvs_handle, "ssid", ssid);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to save SSID!");
    }

    err = nvs_set_str(nvs_handle, "password", password);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to save password!");
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to commit changes in NVS!");
    }

    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Wi-Fi credentials saved to NVS");
    return EXIT_SUCCESS;
}

/**
 * @brief Reads Wi-Fi credentials from NVS
 */
int load_wifi_credentials(char *ssid, size_t ssid_size, char *password, size_t pass_size)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NETWORK_STORAGE_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "No stored Wi-Fi credentials found.");
        return EXIT_FAILURE;
    }

    // Read SSID
    err = nvs_get_str(nvs_handle, "ssid", ssid, &ssid_size);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "No SSID found in NVS, using default.");
        strncpy(ssid, DEFAULT_SSID, ssid_size);
    }

    // Read Password
    err = nvs_get_str(nvs_handle, "password", password, &pass_size);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "No password found in NVS, using default.");
        strncpy(password, DEFAULT_PASS, pass_size);
    }

    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Loaded SSID: %s", ssid);
    return EXIT_SUCCESS;
}

void wifi_init_sta(const char *ssid, const char *password)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {.capable = true, .required = false},
        },
    };

    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s", ssid);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", ssid, password);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}


static int connect_wifi_handler(int argc, char **argv) {
    if (argc != 1) {
        printf("Usage: %s\n(No arguments required; uses pre-configured SSID/Password)\n", argv[0]);
        return 1;
    }



    char ssid[32] = {0};
    char password[64] = {0};
    int res = load_wifi_credentials(ssid, sizeof(ssid), password, sizeof(password));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {.capable = true, .required = false},
        },
    };

    // Safely copy SSID and Password from Params
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    printf("Configuring WiFi with SSID: %s...\n", wifi_config.sta.ssid);

    // Attempt WiFi connection steps with error handling
    esp_err_t ret;

    ret = esp_wifi_disconnect();
    if (ret != ESP_OK && ret != ESP_ERR_WIFI_NOT_INIT) {
        printf("Disconnect failed: %s\n", esp_err_to_name(ret));
        return 1;
    }

    ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (ret != ESP_OK) {
        printf("Set config failed: %s\n", esp_err_to_name(ret));
        return 1;
    }

    ret = esp_wifi_connect();
    if (ret != ESP_OK) {
        printf("Connection failed: %s\n", esp_err_to_name(ret));
        return 1;
    }

    printf("WiFi connection initiated successfully.\n");
    return 0;
}

void register_wifi_connect_command(void) {
    esp_console_cmd_t cmd = {
        .command = "connect_wifi",
        .help = "Connect to WiFi using pre-configured SSID and password",
        .hint = NULL,
        .func = &connect_wifi_handler
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}
