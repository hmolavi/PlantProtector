/**
 * @file esp32_arduino_comm.c
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * @brief Communication interface between ESP32 and Arduino systems.
 *
 * This source file implements SPI communication between ESP32 and Arduino.
 * It defines the functions, data structures, and error handling mechanisms for
 * interfacing an ESP32 with Arduino via SPI, including data chunk encoding and
 * decoding with Hamming error correction.
 *
 * @version 1.0
 * @date 2024-04-13
 *
 * @copyright Copyright (c) 2023
 */

#include "esp32_arduino_comm.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hamming.h"

static const char *TAG = "esp32_arduino_comm.c";

typedef struct {
    const char *name;
    uint8_t code;
    const char *description;
} SPICommandInfo_t;

#define COMM_CMD(name, code, desc) {#name, code, desc},
static const SPICommandInfo_t CommDescriptor[] = {
#include "comm_commands.inc"
    {NULL, 0, NULL}  // MaxCommCmd
};
#undef COMM_CMD

/*-----------------------------------------------------------*/

/* Platform Abstraction Layer */
#ifdef ARDUINO
#include <SPI.h>
#define COMM_SPI_BEGIN() SPI.begin()
#define COMM_DELAY(ms) delay(ms)
#define GET_TIME_MS millis()
#else  // elif defined(ESP_IDF)
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static spi_device_handle_t spi_device;
#define HIGH 1
#define LOW 0
#define COMM_DELAY(ms) vTaskDelay(pdMS_TO_TICKS(ms))
#define GET_TIME_MS esp_timer_get_time() / 1000
#endif

/* Common Defines */
#define COMM_RETRY_COUNT 5
#define COMM_TIMEOUT_MS 10000
#define COMM_RESPONSE_POLL_INTERVAL_MS 100

int CommManager_SendChunk(uint8_t *encoded_chunk)
{
    spi_transaction_t trans = {
        .length = CHUNK_ENCODED_SIZE * 8,
        .tx_buffer = encoded_chunk,
    };

    esp_err_t ret = spi_device_transmit(spi_device, &trans);
    return (ret == ESP_OK) ? COMM_SUCCESS : COMM_SPI_ERROR;
}

int CommManager_Init(void)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = SPI_MOSI,
        .miso_io_num = SPI_MISO,
        .sclk_io_num = SPI_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = CHUNK_ENCODED_SIZE,
        .flags = 0};

    spi_device_interface_config_t devcfg = {
        .mode = 0,                 // SPI mode 0
        .clock_speed_hz = 100000,  // 100 KHz
        .spics_io_num = SPI_SS,
        .queue_size = 1,
    };

    esp_err_t ret;

    /* HSPI (SPI2) and VSPI (SPI3) */
    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        Comm_Log("Failed to initialize SPI bus: %d", ret);
        return EXIT_FAILURE;
    }

    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi_device);
    if (ret != ESP_OK) {
        Comm_Log("Failed to add SPI device: %d", ret);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#ifndef ARDUINO
CommError_t Comm_ExecuteCommand(SPICommands_t action, const char *data)
{
    if (spi_device == NULL) {
        Comm_Log("Comm_ExecuteCommand() Failed. SPI device handle is not initialized");
        return COMM_SPI_ERROR;
    }

    /* Parameter Validation */
    if (action >= MaxCommCmd) {
        Comm_Log("Invalid command");
        return COMM_INVALID_PARAM;
    }

    const size_t dataLen = data ? strnlen(data, DATA_LENGTH + 1) : 0;

    /* Business Logic Validation */
    if (action != COMM_SD_Read && action != COMM_RTC_Read) {
        if (dataLen < 1) {
            Comm_Log("Data required for non-read commands");
            return COMM_INVALID_PARAM;
        }
    }

    Comm_Log("Performing (%s)(0x%02X) action with data (%s)", CommDescriptor[action].name, CommDescriptor[action].code, data ? data : "NULL");

    if (dataLen > DATA_LENGTH) {
        Comm_Log("Data exceeds max length (%d/%d)", dataLen, DATA_LENGTH);
        return COMM_INVALID_PARAM;
    }

    /* Chunk Preparation */
    Chunk_t chunk = {
        .header = CommDescriptor[action].code,
        .crc = 0  // Set by encode_chunk
    };
    memset(chunk.data, ' ', DATA_LENGTH);
    if (data) memcpy(chunk.data, data, dataLen);

    /* Hamming Encoding */
    uint8_t encoded_chunk[CHUNK_ENCODED_SIZE];
    if (encode_chunk(chunk, encoded_chunk)) {
        Comm_Log("Encoding failed");
        return COMM_ENCODING_ERROR;
    }

    // Print the encoded chunk
    for (size_t i = 0; i < CHUNK_ENCODED_SIZE; i++) {
        if (i % 16 == 0) printf("\n");
        printf("0x%02X ", encoded_chunk[i]);
    }
    printf("\n");

    esp_err_t ret;
    printf("Ight here we go\n");

    uint8_t *tx_data = encoded_chunk;
    spi_transaction_t t1 = {
        .tx_buffer = tx_data,
        .length = 56 * 8, // CHUNK_ENCODED_SIZE
    };
    ret = spi_device_transmit(spi_device, &t1);
    ESP_ERROR_CHECK(ret);
    Comm_Log("SPI transfer Sent");
    vTaskDelay(pdMS_TO_TICKS(1000));

    return COMM_SUCCESS;
}
#endif

/*-----------------------------------------------------------*/

static void bits_to_bytes(const int *bits, size_t num_bits, uint8_t *bytes)
{
    memset(bytes, 0, (num_bits + 7) / 8);
    for (size_t i = 0; i < num_bits; i++) {
        if (bits[i]) bytes[i / 8] |= (1 << (7 - (i % 8)));
    }
}

static void bytes_to_bits(const uint8_t *bytes, size_t num_bytes, int *bits)
{
    for (size_t i = 0; i < num_bytes * 8; i++) {
        bits[i] = (bytes[i / 8] >> (7 - (i % 8))) & 1;
    }
}

/**
 * @brief Computes the CRC-16-CCITT checksum for the given data.
 *
 * This function calculates the CRC-16-CCITT checksum for a given array of data
 * using the polynomial 0x1021. The initial value of the CRC is set to 0xFFFF.
 *
 * @param data Pointer to the data array for which the CRC is to be computed.
 * @param len Length of the data array.
 * @return The computed CRC-16-CCITT checksum.
 */
static uint16_t compute_crc(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            }
            else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/*-----------------------------------------------------------*/

int encode_chunk(Chunk_t chunk, uint8_t *encoded_chunk)
{
    /* 1. Compute CRC for header + data */
    uint8_t crc_data[DATA_LENGTH + 1] = {chunk.header};
    memcpy(crc_data + 1, chunk.data, DATA_LENGTH);
    chunk.crc = compute_crc(crc_data, sizeof(crc_data));

    /* 2. Convert entire chunk to bit array */
    int chunk_bits[sizeof(Chunk_t) * 8];  // 32 bytes * 8 = 256 bits
    bytes_to_bits((uint8_t *)&chunk, sizeof(Chunk_t), chunk_bits);

    /* 3. Hamming(7,4) encode in 4-bit segments => 64 segments => 448 bits */
    int encoded_bits[CHUNK_ENCODED_SIZE * 8];
    hamming_encode_74(chunk_bits, sizeof(Chunk_t) * 8, encoded_bits);

    /* 4. Convert back to bytes */
    bits_to_bytes(encoded_bits, CHUNK_ENCODED_SIZE * 8, encoded_chunk);

    return EXIT_SUCCESS;
}

/*-----------------------------------------------------------*/

int decode_chunk(uint8_t *encoded_chunk, Chunk_t *decoded_chunk)
{
    /* 1. Convert encoded bytes to bits */
    int encoded_bits[CHUNK_ENCODED_SIZE * 8];  // 56 bytes * 8 = 448 bits
    bytes_to_bits(encoded_chunk, CHUNK_ENCODED_SIZE, encoded_bits);

    /* 2. Hamming decode in 4-bit segments (448 bits -> 256 bits) */
    int decoded_bits[sizeof(Chunk_t) * 8];  // 256 bits
    hamming_decode_74(encoded_bits, sizeof(Chunk_t) * 8, decoded_bits);

    /* 3. Convert bits back to chunk */
    bits_to_bytes(decoded_bits, sizeof(Chunk_t) * 8, (uint8_t *)decoded_chunk);

    /* 4. Verify CRC */
    uint8_t check_data[DATA_LENGTH + 1] = {decoded_chunk->header};
    memcpy(check_data + 1, decoded_chunk->data, DATA_LENGTH);

    uint16_t calculated_crc = compute_crc(check_data, sizeof(check_data));
    return (calculated_crc == decoded_chunk->crc) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*-----------------------------------------------------------*/

int Comm_Printf(const char *fmt, ...)
{
    int rc;
    char buf[400];
    va_list args;
    va_start(args, fmt);
    rc = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

#ifndef ARDUINO
    printf("%s", buf);
#else
    Serial.print(buf);
#endif

    return rc;
}

/*-----------------------------------------------------------*/

int Comm_Log(const char *fmt, ...)
{
    int rc;
    char buf[400];
    va_list args;
    va_start(args, fmt);
    rc = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

#ifndef ARDUINO
    printf("%s: %s\n", TAG, buf);
#else
    Serial.print(TAG);
    Serial.print(": ");
    Serial.println(buf);
#endif

    return rc;
}

/*-----------------------------------------------------------*/