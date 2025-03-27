/**
 *  Header Commands                     Hex      Binary
 * --------------------------------------------------------
 *  SD Card
 *      Read                            0x10    0001 0000
 *      Append                          0x11    0001 0001
 *      Newline, timestamp then append  0x12    0001 0011
 *  
 * RTC
 *      Read                            0x20    0010 0000
 *      Set                             0x21    0010 0001
 *  
 *  ACK                                 0xFD    1111 1101
 *  NACK                                0xFE    1111 1110
 *  Abort                               0xFF    1111 1111
 * ---------------------------------------------------------
 * 
 * Theres 2 extra header commands for read operations of 
 *  SD Card reply and RTC which is the same code but 
 *  XOR'ed with 0x80 (i.e. 0x90 for SD Card read and
 *  0xA0 for RTC read). These reply commands are used by
 *  the Arduino to reply to the ESP32.
 * 
 *     ESP32        --- RTC_Read -->             Arduino
 *     ESP32    <-- RTC_Read_R with data ---     Arduino
 *              
 */


#ifndef COMM_CMD
#define COMM_CMD(name, code, description)
#endif

COMM_CMD(SD_Read,     0x10, "Read data from SD card")
COMM_CMD(SD_Append,   0x11, "Append data to SD card")
COMM_CMD(SD_lnAppend, 0x12, "Append data with newline and timestamp to SD card")

COMM_CMD(RTC_Read,    0x20, "Read data from RTC")
COMM_CMD(RTC_Set,     0x21, "Set RTC time")

COMM_CMD(SD_Read_R,   0x90, "Reply for SD card read operation")
COMM_CMD(RTC_Read_R,  0xA0, "Reply for RTC read operation")

COMM_CMD(ACK,         0xFD, "Acknowledge command")
COMM_CMD(NACK,        0xFE, "Negative acknowledge command")
COMM_CMD(Abort,       0xFF, "Abort operation")

#undef COMM_CMD