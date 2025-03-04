/**
 * @file spi_arduino.c
 * @brief SPI Slave Code for Arduino
 *
 * This file contains the implementation of an SPI slave for Arduino. The code
 * handles the reception of data chunks over SPI, verifies their integrity using
 * CRC-16-CCITT, decodes the data using Hamming code, and processes the received
 * data. The main components of the code include:
 *
 * - CRC computation function
 * - SPI interrupt service routine
 * - Chunk handling function
 * - Byte-to-bit conversion function
 *
 * The code is designed to work with a chunk size of 32 bytes, where the last two
 * bytes are used for CRC. The received data is stored in a global buffer and
 * processed once the final chunk is received.
 *
 * @note The code assumes that the received data is encoded using Hamming code
 * and that the total number of chunks is known.
 *
 */

#include <Arduino.h>
#include <SPI.h>

#define CHUNK_SIZE 32
#define DATA_PART_SIZE 28
#define HEADER_INDEX 0
#define SEQ_INDEX 1
#define DATA_START_INDEX 2
#define CRC_START_INDEX 30

volatile bool chunkReceived = false;
uint8_t receivedChunk[CHUNK_SIZE];
uint8_t expectedSeq = 0;
uint8_t decodedBuffer[255];  // max expected message size
size_t decodedPos = 0;

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
 * @brief Calculate the syndrome for the given encoded Hamming code data.
 *
 * This function calculates the syndrome for error detection in a Hamming code.
 * The syndrome is used to identify the position of a single-bit error in the
 * encoded data. The function depends on the `parity_check` function to compute
 * the parity for each parity bit position.
 *
 * - Dependant on parity_check()
 *
 * @param n The number of bits in the encoded_data array.
 * @param encoded_data Pointer to the array containing the encoded Hamming code data.
 * @return The calculated syndrome, which indicates the position of a single-bit error.
 */
int calculate_syndrome(int n, int *encoded_data)
{
    int syndrome = 0;
    int p = 0;
    // Determine the number of parity bits needed (r) such that 2^r >= n + r + 1
    // For simplicity, iterate while 2^p <= n
    while ((1 << p) <= n) {
        int parity_pos = (1 << p) - 1;  // 0-based position of parity bit
        if (parity_pos >= n) break;
        // Compute the parity check for this p
        int check = parity_check(n, encoded_data, p);
        // If the check is non-zero, set the corresponding syndrome bit
        syndrome |= (check << p);
        p++;
    }
    return syndrome;
}

/**
 * @brief Decodes encoded data using Hamming code.
 *
 * This function takes encoded data that uses Hamming code for error detection
 * and correction, calculates the syndrome to detect any errors, corrects the
 * error if detected, and then extracts the original data bits from the encoded
 * data.
 *
 * - Dependant on calculate_syndrome()
 * - Dependant on parity_check()
 *
 * @param encoded_data Pointer to the array of encoded data bits.
 * @param n The number of bits in the encoded data.
 * @param decoded_data Pointer to the array where the decoded data bits will be stored.
 *
 * The function performs the following steps:
 * 1. Calculates the syndrome to detect errors in the encoded data.
 * 2. If an error is detected (syndrome is non-zero), it corrects the error by flipping the incorrect bit.
 * 3. Determines the number of parity bits used in the encoded data.
 * 4. Extracts the original data bits from the encoded data, ignoring the parity bits.
 */
void hamming_decode(int *encoded_data, int n, int *decoded_data)
{
    // Calculate the syndrome to detect errors
    int syndrome = calculate_syndrome(n, encoded_data);

    // Correct the error if syndrome is non-zero
    if (syndrome != 0) {
        int error_pos = syndrome - 1;  // Convert to 0-based index
        if (error_pos < n) {
            encoded_data[error_pos] ^= 1;  // Flip the incorrect bit
        }
    }

    // Determine the number of parity bits (r) used in the encoded data
    int r = 0;
    while ((1 << r) <= n) {
        r++;
    }

    // Extract data bits
    int data_bits = n - r;
    int decoded_index = 0;
    for (int i = 0; i < n; i++) {
        // Check if the current position is a data bit and not a parity position
        if ((i & (i + 1)) != 0) {
            decoded_data[decoded_index++] = encoded_data[i];
        }
    }
}

/**
 * @brief Computes the CRC-16-CCITT checksum for the given data.
 *        Identical on both sending and recieving sides.
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
 * @brief Handles the received chunk of data over SPI.
 *
 * This function performs the following steps:
 * 1. Verifies the CRC of the received chunk.
 * 2. Converts the encoded data to bits.
 * 3. Decodes each 7-bit segment using Hamming code.
 * 4. Converts the decoded bits back to bytes.
 * 5. Stores the decoded data in a buffer.
 * 6. Sends an ACK or NACK response over SPI.
 * 7. Processes the final chunk if it is the last one.
 *
 * The function expects the received chunk to be in the global variable `receivedChunk`.
 * The decoded data is stored in the global buffer `decodedBuffer`.
 * The function uses the global variables `expectedSeq`, `decodedPos`, and `total_chunks`
 * to manage the sequence and position of the data.
 *
 * @note This function assumes that the received chunk is 32 bytes long, with the last
 * two bytes being the CRC.
 */
void handleReceivedChunk()
{
    // Verify chunk CRC
    uint16_t received_crc = (receivedChunk[30] << 8) | receivedChunk[31];
    uint16_t computed_crc = compute_crc(receivedChunk, 30);

    if (received_crc != computed_crc || receivedChunk[SEQ_INDEX] != expectedSeq) {
        SPI.transfer(0xFF);  // NACK
        return;
    }

    // Convert encoded data to bits
    int encoded_bits[28 * 8];
    bytes_to_bits(&receivedChunk[DATA_START_INDEX], 28, encoded_bits);

    // Decode each 7-bit segment
    int decoded_bits[32 * 4];       // 28 bytes → 32*4 bits
    for (int i = 0; i < 32; i++) {  // 32 nibbles per chunk
        int encoded_segment[7];
        memcpy(encoded_segment, &encoded_bits[i * 7], 7 * sizeof(int));
        hamming_decode(encoded_segment, 7, &decoded_bits[i * 4]);
    }

    // Convert bits to bytes
    uint8_t decoded_data[16];
    for (int i = 0; i < 16; i++) {
        decoded_data[i] = 0;
        for (int j = 0; j < 4; j++) {  // High nibble
            decoded_data[i] |= decoded_bits[i * 8 + j] << (7 - j);
        }
        for (int j = 0; j < 4; j++) {  // Low nibble
            decoded_data[i] |= decoded_bits[i * 8 + 4 + j] << (3 - j);
        }
    }

    // Store in buffer
    memcpy(&decodedBuffer[decodedPos], decoded_data, 16);
    decodedPos += 16;

    SPI.transfer(0x00);  // ACK
    expectedSeq++;

    // Final chunk processing
    if (receivedChunk[SEQ_INDEX] == total_chunks - 1) {
        uint16_t received_crc = (decodedBuffer[decodedPos - 2] << 8) | decodedBuffer[decodedPos - 1];
        uint16_t computed_crc = compute_crc(decodedBuffer, decodedPos - 2);

        if (received_crc == computed_crc) {
            // Process valid data
        }
        decodedPos = 0;
        expectedSeq = 0;
    }
}

/**
 * @brief Convert an array of bytes to an array of bits.
 *        Used to decode messages on the receiving end
 *
 * This function takes an array of bytes and converts each byte into its
 * corresponding bits, storing the result in an array of integers.
 *
 * @param bytes Pointer to the array of bytes to be converted.
 * @param num_bytes The number of bytes in the input array.
 * @param bits Pointer to the array of integers where the resulting bits will be stored.
 *             The array should be large enough to hold num_bytes * 8 elements.
 */
void bytes_to_bits(const uint8_t *bytes, size_t num_bytes, int *bits)
{
    for (size_t i = 0; i < num_bytes * 8; i++) {
        bits[i] = (bytes[i / 8] >> (7 - (i % 8))) & 1;
    }
}

/**
 * @brief Interrupt Service Routine (ISR) for SPI communication.
 *
 * This function is called when an SPI interrupt occurs. It reads a chunk of data
 * from the SPI bus and stores it in the receivedChunk array. Once the chunk is
 * received, it sets the chunkReceived flag to true.
 */
void SPI_ISR()
{
    for (int i = 0; i < CHUNK_SIZE; i++) {
        receivedChunk[i] = SPI.transfer(0x00);
    }
    chunkReceived = true;
}

void setup()
{
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    SPCR |= _BV(SPE);
    SPI.attachInterrupt();
}

void loop()
{
    if (chunkReceived) {
        chunkReceived = false;
        handleReceivedChunk();
    }
}
