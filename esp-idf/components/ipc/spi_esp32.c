/**
 * @file esp32.c
 * @brief SPI Master Code for ESP32
 *
 * This file contains the implementation of SPI communication for the ESP32.
 * It includes functions for computing CRC, converting between bits and bytes,
 * and sending messages over SPI with CRC and Hamming encoding.
 *
 * @details
 * The main functionalities provided in this file are:
 * - Initialization of SPI communication in the `setup` function.
 * - Sending messages over SPI in the `loop` function.
 * - Computing CRC for data integrity verification.
 * - Converting data to nibbles and applying Hamming encoding for error detection and correction.
 * - Splitting encoded messages into chunks and sending them over SPI.
 *
 * The main functions are:
 * - `compute_crc`: Computes the CRC for a given data array.
 * - `send_spi_message`: Sends a message over SPI with CRC and Hamming encoding.
 * - `bits_to_bytes`: Converts an array of bits to an array of bytes.
 *
 * @note
 * The SPI communication is configured with a clock speed of 1 MHz, MSB first, and SPI mode 0.
 * The chunk size for SPI communication is 32 bytes, with 28 bytes for data and 4 bytes for header and CRC.
 *
 */

#include <Arduino.h>
#include <SPI.h>

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define SPI_SS 5

#define CHUNK_SIZE 32
#define DATA_PART_SIZE 28
#define HEADER_INDEX 0
#define SEQ_INDEX 1
#define DATA_START_INDEX 2
#define CRC_START_INDEX 30


/**
 * @brief Sends a message over SPI with CRC and Hamming encoding.
 *
 * This function takes a header and data, computes a CRC for the data,
 * converts the data to nibbles, applies Hamming encoding to each nibble,
 * and sends the encoded message in 28-byte chunks over SPI. Each chunk
 * includes a header, sequence number, data, and CRC.
 *
 * @param header The command header to send.
 * @param data Pointer to the data to be sent.
 * @param data_len Length of the data to be sent.
 */
void send_spi_message(uint8_t header, const uint8_t *data, size_t data_len)
{
    // Original message with CRC
    uint8_t message[data_len + 2];
    memcpy(message, data, data_len);
    uint16_t data_crc = compute_crc(data, data_len);
    message[data_len] = (uint8_t)(data_crc >> 8);
    message[data_len + 1] = (uint8_t)(data_crc & 0xFF);
    size_t original_len = data_len + 2;

    // Convert message to nibbles (4-bit chunks)
    size_t num_nibbles = original_len * 2;
    int *nibble_bits = (int *)malloc(num_nibbles * 4 * sizeof(int));

    // Extract bits (MSB first)
    for (size_t i = 0; i < original_len; i++) {
        uint8_t byte = message[i];
        for (int j = 0; j < 4; j++) {
            nibble_bits[i * 8 + j] = (byte >> (7 - j)) & 1;      // High nibble
            nibble_bits[i * 8 + 4 + j] = (byte >> (3 - j)) & 1;  // Low nibble
        }
    }

    // Hamming encode each nibble
    int *encoded_bits = (int *)malloc(num_nibbles * 7 * sizeof(int));
    for (size_t i = 0; i < num_nibbles; i++) {
        int nibble[4] = {
            nibble_bits[i * 4],
            nibble_bits[i * 4 + 1],
            nibble_bits[i * 4 + 2],
            nibble_bits[i * 4 + 3]};
        int encoded[7];
        hamming_encode(nibble, 4, encoded);
        memcpy(&encoded_bits[i * 7], encoded, 7 * sizeof(int));
    }

    // Convert encoded bits to bytes
    size_t encoded_len = (num_nibbles * 7 + 7) / 8;
    uint8_t *encoded_message = (uint8_t *)malloc(encoded_len);
    bits_to_bytes(encoded_bits, num_nibbles * 7, encoded_message);

    // Split into 28-byte chunks and send
    size_t num_chunks = (encoded_len + 27) / 28;
    for (size_t seq = 0; seq < num_chunks; seq++) {
        uint8_t chunk[CHUNK_SIZE];
        chunk[HEADER_INDEX] = header;
        chunk[SEQ_INDEX] = seq;

        size_t chunk_start = seq * 28;
        size_t chunk_len = (seq == num_chunks - 1) ? encoded_len - chunk_start : 28;
        memcpy(&chunk[DATA_START_INDEX], &encoded_message[chunk_start], chunk_len);

        // Compute chunk CRC
        uint16_t chunk_crc = compute_crc(chunk, CHUNK_SIZE - 2);
        chunk[CRC_START_INDEX] = (uint8_t)(chunk_crc >> 8);
        chunk[CRC_START_INDEX + 1] = (uint8_t)(chunk_crc & 0xFF);

        // Send chunk
        uint8_t ack;
        do {
            SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
            digitalWrite(SPI_SS, LOW);
            for (int i = 0; i < CHUNK_SIZE; i++) SPI.transfer(chunk[i]);
            ack = SPI.transfer(0x00);
            digitalWrite(SPI_SS, HIGH);
            SPI.endTransaction();
        } while (ack != 0x00);
    }

    free(nibble_bits);
    free(encoded_bits);
    free(encoded_message);
}


void setup()
{
    pinMode(SPI_SS, OUTPUT);
    digitalWrite(SPI_SS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_SS);
}

void loop()
{
    uint8_t data[] = "Hello, Arduino! Testing chunked SPI communication.";
    send_spi_message(0x01, data, strlen((char *)data));
    delay(1000);
}