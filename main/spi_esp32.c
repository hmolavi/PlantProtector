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
 * @brief Calculate the parity for a given parity bit position.
 *
 * This function calculates the parity for a specified parity bit position
 * in the data array. The parity bit position is determined by the index p.
 *
 * @param n The total number of bits in the encoded data.
 * @param data Pointer to the array containing the data bits.
 * @param p The index of the parity check (0 for P1, 1 for P2, etc.).
 * @return The calculated parity (0 or 1).
 */
int parity_check(int n, int *data, int p)
{
    int mask = 1 << p;  // 1-based position of the parity bit (2^p)
    int sum = 0;
    // Iterate over all bits starting from the current parity bit's position
    for (int i = mask - 1; i < n; i++) {
        // Check if the (i+1)-th bit has the p-th bit set (1-based position)
        if (((i + 1) & mask) != 0) {
            sum ^= data[i];
        }
    }
    return sum;
}

/**
 * @brief Encode data using Hamming code.
 *
 * This function encodes the given data using Hamming code, which includes
 * calculating and setting the necessary parity bits. The encoded data is
 * stored in the provided encoded_data array.
 *
 * - Dependant on parity_check()
 *
 * @param data Pointer to the array containing the data bits to be encoded.
 * @param data_bits The number of data bits in the input array.
 * @param encoded_data Pointer to the array where the encoded data will be stored.
 */
void hamming_encode(int *data, int data_bits, int *encoded_data)
{
    // Calculate the required number of parity bits (r)
    int r = 0;
    while ((1 << r) < (data_bits + r + 1)) {
        r++;
    }
    int n = data_bits + r;

    // Initialize encoded data to 0
    for (int i = 0; i < n; i++) {
        encoded_data[i] = 0;
    }

    // Place data bits in non-parity positions
    int j = 0;
    for (int i = 0; i < n; i++) {
        // Check if current position is a parity bit position (i+1 is a power of 2)
        if ((i & (i + 1)) == 0) {
            continue;  // Skip parity positions
        }
        if (j < data_bits) {  // Ensure we don't exceed data array bounds
            encoded_data[i] = data[j];
            j++;
        }
    }

    // Calculate and set the parity bits
    for (int p = 0; p < r; p++) {
        int parity_pos = (1 << p) - 1;  // 0-based index
        if (parity_pos < n) {           // Ensure parity position is within bounds
            encoded_data[parity_pos] = parity_check(n, encoded_data, p);
        }
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
uint16_t compute_crc(const uint8_t *data, size_t len)
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

/**
 * @brief Convert an array of bits to an array of bytes.
 *        Used to encode messages from the sender
 *
 * This function takes an array of bits and converts it into an array of bytes.
 * Each byte in the output array will represent 8 bits from the input array.
 *
 * @param bits Pointer to the input array of bits.
 * @param num_bits The number of bits in the input array.
 * @param bytes Pointer to the output array of bytes. The size of this array
 *              should be at least (num_bits + 7) / 8 bytes.
 */
void bits_to_bytes(const int *bits, size_t num_bits, uint8_t *bytes)
{
    memset(bytes, 0, (num_bits + 7) / 8);
    for (size_t i = 0; i < num_bits; i++) {
        if (bits[i]) bytes[i / 8] |= (1 << (7 - (i % 8)));
    }
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