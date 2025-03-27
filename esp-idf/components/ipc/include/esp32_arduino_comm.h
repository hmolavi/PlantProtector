/** esp32_arduino_comm.h
 *
 */

#ifndef __ESP32_ARDUINO_COMM_H__
#define __ESP32_ARDUINO_COMM_H__

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ISTHISESP32 1

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

/* Dont change these lol, values in Bytes */
#define DATA_LENGTH 29
#define CHUNK_SIZE 32
#define SIZE_CRC 2
/* Encoded size calculated based off Hamming(7,4) */
#define CHUNK_ENCODED_SIZE (CHUNK_SIZE*7)/4

/* Ports */
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define SPI_SS 5


/** Data chunk
 *
 * Fixed datalength of 29 bytes,
 * makes the chunk size 32 bytes
 *
 */
typedef struct Chunk_s {
    uint8_t header;
    uint8_t data[DATA_LENGTH];
    uint16_t crc;
} Chunk_t;

/* Communication commands */
#define COMM_CMD(name, code, desc) COMM_##name,
typedef enum IPCCommands_e {
#include "comm_commands.inc"
    MaxCommCmd
} IPCCommands_t;
#undef COMM_CMD

// typedef enum CommMode_e {
//     MASTER,
//     SLAVE
// } CommMode_t
int CommManager_Init(void);

/* Printf style output to the approperiate console */
int Comm_Printf(const char *fmt, ...);

int Comm_Log(const char *fmt, ...);

int Comm_ExecuteCommand(IPCCommands_t action, char *data);

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
uint16_t compute_crc(const uint8_t *data, size_t len);

int encode_chunk(Chunk_t chunk, uint8_t *encoded_chunk);

int decode_chunk(uint8_t *encoded_chunk, Chunk_t *decoded_chunk);

#ifdef __cplusplus
}
#endif

#endif  // __ESP32_ARDUINO_COMM_H__