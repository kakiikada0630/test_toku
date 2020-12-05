#ifndef BIN_FORMAT_H
#define BIN_FORMAT_H

#define BLOCK_SIZE       304
#define BLOCK_TICK       8
#define BLOCK_PWM        12
#define BLOCK_SPI        40
#define BLOCK_UART       140
#define BLOCK_LIN        270
#define BLOCK_DAC        290
#define BLOCK_SW         300

#define SW_VBU_STATUS    0x00000001
#define SW_IG1_STATUS    0x00000002
#define SW_TURNS_STATUS  0x00000004
#define SW_HLBKUP_STATUS 0x00000008


#endif // #define BIN_FORMAT_H
