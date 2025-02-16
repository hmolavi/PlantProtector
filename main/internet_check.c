///@file internet_check.c
///@author Hossein Molavi (hmolavi@uwaterloo.ca)
///@brief Checking to see if we have internet in addition to wifi by pinging one of the tech giants' servers
///@version 1.0
///@date 2025-02-14
///
///@copyright Copyright (c) 2025
///

#include "include/internet_check.h"

#include <stdlib.h>

#include "esp_log.h"

static const char *TAG = "internet_check.c";

int check_connection_to_host(const char *host)
{
    struct sockaddr_in server_addr;
    char response[1024];

    // Resolve hostname
    struct hostent *server = gethostbyname(host);
    if (!server) {
        ESP_LOGE(TAG, "Failed to resolve host: %s", host);
        return EXIT_FAILURE;
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
        return EXIT_FAILURE;
    }

    struct timeval timeout = {5, 0};  // 5 seconds timeout
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Failed to connect to server: %s", host);
        close(sockfd);
        return EXIT_FAILURE;
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
        return EXIT_FAILURE;
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
            return EXIT_SUCCESS;
        }
    }

    ESP_LOGE(TAG, "No response or invalid HTTP response from %s", host);
    return EXIT_FAILURE;
}

int check_internet_connection()
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
        if (check_connection_to_host(reliable_hosts[i]) == EXIT_SUCCESS) {
            ESP_LOGI(TAG, "Internet connection verified with %s", reliable_hosts[i]);
            return EXIT_SUCCESS;
        }
    }

    ESP_LOGE(TAG, "Failed to verify internet connection.");
    return EXIT_FAILURE;
}
