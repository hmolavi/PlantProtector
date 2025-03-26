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
 */


#ifndef SPICMD
#define SPICMD(...)
#endif


SPICMD(SD_Read,     0x10)
SPICMD(SD_Read_R,   0x90)
SPICMD(SD_Append,   0x11)
SPICMD(SD_lnAppend, 0x12)

SPICMD(RTC_Read,    0x20)
SPICMD(RTC_Read_R,  0xA0)
SPICMD(RTC_Set,     0x21)

SPICMD(ACK,         0xFD)
SPICMD(NACK,        0xFE)
SPICMD(Abort,       0xFF)


#undef SPICMD