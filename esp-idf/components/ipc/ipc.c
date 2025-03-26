



#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "include/ipc.h"

/**
 * Fixed datalength of 29 bytes,
 * makes the chunk size 32 bytes 
 *  
 */ 
#define DATA_LENGTH 29

/**
 *  Header Commands                     Hex      Binary
 * --------------------------------------------------------
 *  SD Card
 *      Read                            0x10    0001 0000
 *      Append                          0x11    0001 0001
 *      Newline, timestamp then append  0x12    0001 0011
 *  RTC
 *      Read                            0x20    0010 0000
 *      Set                             0x21    0010 0001
 *  
 *  Ack                                 0xFD    1111 1101
 *  NACK                                0xFE    1111 1110
 *  Abort                               0xFF    1111 1111
 * ---------------------------------------------------------
 * 
 * Theres 2 extra header commands for read operations of 
 *  SD Card reply and RTC which is the same code but 
 *  XOR'ed with 0x80 (i.e. 0x90 for SD Card read and
 *  0xA0 for RTC read). These reply commands are used by
 *  the Arduino to reply to the ESP32.
 */

typedef enum {
    SD_CARD_READ = 0x10,
    SD_CARD_APPEND = 0x11,
    SD_CARD_NEWLINE_APPEND = 0x12,
    RTC_READ = 0x20,
    RTC_SET = 0x21,
    ACK = 0xFD,
    NACK = 0xFE,
    ABORT = 0xFF
} IPCCommands_t;

typedef struct {
    IPCCommands_t command;
    const char *description;
} IPCCommandsAutoComplete;

IPCCommandsAutoComplete ipcAutoComplete[] = {
    {SD_CARD_READ, "SD Card Read"},
    {SD_CARD_APPEND, "SD Card Append"},
    {SD_CARD_NEWLINE_APPEND, "SD Card Newline, timestamp then append"},
    {RTC_READ, "RTC Read"},
    {RTC_SET, "RTC Set"},
    {ACK, "Acknowledge"},
    {NACK, "Not Acknowledge"},
    {ABORT, "Abort"}
};

void performAction(IPCCommands_t action){
    
    return;
}

/**
 * Total 32 bytes in one chunk
 */
typedef struct chunk_s {
    uint8_t command;
    uint8_t data[DATA_LENGTH];
    uint16_t crc;
} chunk_t;


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
 * process for sending commands:
 * Create 
 * After Hamming(7,4) will be 56 bytes
 */