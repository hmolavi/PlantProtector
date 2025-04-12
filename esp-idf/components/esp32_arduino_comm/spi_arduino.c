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
#ifdef ARDUINO
#include <Arduino.h>
#include <SPI.h>

#include "esp32_arduino_comm.h"

#define CHUNK_SIZE 32
#define DATA_PART_SIZE 28
#define HEADER_INDEX 0
#define SEQ_INDEX 1
#define DATA_START_INDEX 2
#define CRC_START_INDEX 30

volatile bool chunkReceived = false;
uint8_t receivedChunk[CHUNK_ENCODED_SIZE];
uint8_t expectedSeq = 0;
uint8_t decodedBuffer[255];  // max expected message size
size_t decodedPos = 0;

void handleReceivedChunk_old()
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

void handleReceivedChunk()
{
    Chunk_t chunk;

    /* Decode and process */
    if (decode_chunk(receivedChunk, &chunk) == EXIT_SUCCESS) {
        SPI.transfer(COMM_ACK);

        // Null-terminate and print data
        char received_data[DATA_LENGTH + 1];
        memcpy(received_data, chunk.data, DATA_LENGTH);
        received_data[DATA_LENGTH] = '\0';

        // for now j print
        Comm_Printf(received_data);
    }
    else {
        SPI.transfer(COMM_NACK);
        Comm_Printf("CRC Error in received data\n");
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
    // Static index to track the current position in the chunk
    static uint8_t byteCount = 0;

    receivedChunk[byteCount++] = SPDR;

    if (byteCount >= CHUNK_SIZE) {
        byteCount = 0;
        chunkReceived = true;
    }
}

void setup()
{
    SPI.begin();             // Initialize SPI in slave mode
    pinMode(SPI_SS, INPUT);  // Set MISO as output for SPI slave
    SPCR |= _BV(SPE);
    SPI.attachInterrupt();
}

void loop()
{
    if (chunkReceived) {
        Serial.println("HAHA");
        chunkReceived = false;
        handleReceivedChunk();
    }
}
#endif