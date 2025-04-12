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

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

/* Dont change these lol, values in Bytes */
#define DATA_LENGTH 29
#define CHUNK_SIZE 32
#define SIZE_CRC 2
/* Encoded size calculated based off Hamming(7,4) */
#define CHUNK_ENCODED_SIZE (CHUNK_SIZE * 7) / 4

/* SPI Ports */
#ifdef ARDUINO
#define SPI_SS 10
#else
#define SPI_SCK 36
#define SPI_MISO 37
#define SPI_MOSI 35
#define SPI_SS 45
#endif

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
typedef enum SPICommands_e {
#include "comm_commands.inc"
    MaxCommCmd
} SPICommands_t;
#undef COMM_CMD

/* Error Codes */
typedef enum CommError_e {
    COMM_SUCCESS = 0,
    COMM_INVALID_PARAM,
    COMM_ENCODING_ERROR,
    COMM_CRC_ERROR,
    COMM_TIMEOUT,
    COMM_SPI_ERROR
} CommError_t;

int CommManager_Init(void);

/* Printf style output to the approperiate console */
int Comm_Printf(const char *fmt, ...);

int Comm_Log(const char *fmt, ...);

int encode_chunk(Chunk_t chunk, uint8_t *encoded_chunk);

int decode_chunk(uint8_t *encoded_chunk, Chunk_t *decoded_chunk);

#ifndef ARDUINO
CommError_t Comm_ExecuteCommand(SPICommands_t action, const char *data);
#endif

#ifdef __cplusplus
}
#endif

#endif  // __ESP32_ARDUINO_COMM_H__