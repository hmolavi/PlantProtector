///@file wifi.c
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///
///@version 1.0
///@date 2025-02-13
///
///@copyright Copyright (c) 2025
///

#include "include/wifi.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../components/library/include/param_manager.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define MAX_RETRIES 2

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about
 * two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG = "wifi.c";

static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAX_RETRIES) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying connection (%d/%d)", s_retry_num, MAX_RETRIES);
        }
        else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static int check_connection_to_host(const char *host)
{
    struct sockaddr_in server_addr;
    char response[1024];

    // Resolve hostname
    struct hostent *server = gethostbyname(host);
    if (!server) {
        ESP_LOGE(TAG, "Failed to resolve host: %s", host);
        return ESP_FAIL;
    }

    // Setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Create and connect socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        ESP_LOGE(TAG, "Failed to create socket.");
        return ESP_FAIL;
    }

    struct timeval timeout = {5, 0};  // 5 seconds timeout
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Failed to connect to server: %s", host);
        close(sockfd);
        return ESP_FAIL;
    }

    // Send HTTP request
    char request[512];
    snprintf(request, sizeof(request),
             "GET / HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n"
             "\r\n",
             host);

    if (send(sockfd, request, strlen(request), 0) < 0) {
        ESP_LOGE(TAG, "Failed to send HTTP request to server: %s", host);
        close(sockfd);
        return ESP_FAIL;
    }

    // Receive response
    int bytes_received = recv(sockfd, response, sizeof(response) - 1, 0);
    close(sockfd);

    if (bytes_received > 0) {
        response[bytes_received] = '\0';

        // Extract HTTP status code
        char *response_code = strstr(response, "HTTP/1.1");
        if (response_code) {
            response_code += 9;  // Skip "HTTP/1.1 "
            char *end_of_code = strstr(response_code, "\r\n");
            if (end_of_code)
                *end_of_code = '\0';

            // As long as they respond, dont care what the HTTP is
            // ESP_LOGI(TAG, "%s responded with HTTP status: %s", host, response_code);
            return ESP_OK;
        }
    }

    ESP_LOGE(TAG, "No response or invalid HTTP response from %s", host);
    return ESP_FAIL;
}

static int check_internet_connection()
{
    const char *reliable_hosts[] = {
        "www.amazon.com",
        "www.google.com",
        "www.microsoft.com",
        "www.apple.com",
        "www.cloudflare.com",
        "www.akamai.com",
        "www.facebook.com"};
    size_t num_hosts = sizeof(reliable_hosts) / sizeof(reliable_hosts[0]);

    for (size_t i = 0; i < num_hosts; i++) {
        if (check_connection_to_host(reliable_hosts[i]) == ESP_OK) {
            ESP_LOGI(TAG, "Internet connection verified with %s", reliable_hosts[i]);
            return ESP_OK;
        }
    }

    ESP_LOGE(TAG, "Failed to verify internet connection.");
    return ESP_FAIL;
}

void Wifi_InitSta(void)
{
    wifiConnected = false;
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers
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

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi STA mode initialized");
}

void Wifi_TryConnect(void)
{
    const char *ssid = Param_GetSsid(NULL);
    const char *password = Param_GetPassword(NULL);

    if (!ssid || !password || strlen(ssid) == 0) {
        ESP_LOGE(TAG, "Invalid WiFi credentials");
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        return;
    }

    if (wifiConnected) {
        ESP_LOGI(TAG, "Disconnecting from current AP");
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Configure WiFi
    wifi_config_t wifi_config = {
        .sta =
            {
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                .pmf_cfg = {.capable = true, .required = false},
            },
    };

    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // Reset connection state and attempt to connect
    s_retry_num = 0;
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
    ESP_ERROR_CHECK(esp_wifi_connect());

    // Wait for connection outcome
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to AP: %s", ssid);
        if (check_internet_connection() == ESP_OK) {
            wifiConnected = true;
        }
        else {
            ESP_LOGW(TAG, "Connected to AP but no internet access");
            wifiConnected = false;
        }
    }
    else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to SSID: %s", ssid);
        wifiConnected = false;
    }
}