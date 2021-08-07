#ifndef BIN_FORMAT_H
#define BIN_FORMAT_H

// 各データ領域サイズ(4の倍数とする事!!)
#define BLOCK_START_SIZE 8
#define TICK_SIZE        4
#define PWM_SIZE         28
#define SPI_SIZE         100
#define UART_SIZE        152
#define LIN_SIZE         60
#define CAN_SIZE         20
#define DAC_SIZE         10
#define SW_SIZE          4
#define VER_SIZE         12

// 各データ領域の先頭アドレス
#define BLOCK_TICK       BLOCK_START_SIZE
#define BLOCK_PWM        BLOCK_TICK + TICK_SIZE
#define BLOCK_SPI        BLOCK_PWM  + PWM_SIZE
#define BLOCK_UART       BLOCK_SPI  + SPI_SIZE
#define BLOCK_LIN        BLOCK_UART + UART_SIZE
#define BLOCK_CAN        BLOCK_LIN  + LIN_SIZE
#define BLOCK_DAC        BLOCK_CAN  + CAN_SIZE
#define BLOCK_SW         BLOCK_DAC  + DAC_SIZE
#define BLOCK_VER        BLOCK_SW   + SW_SIZE
#define BLOCK_SIZE       BLOCK_VER  + VER_SIZE

#define SW_VBU_STATUS    0x00000001
#define SW_IG1_STATUS    0x00000002
#define SW_TURNS_STATUS  0x00000004
#define SW_HLBKUP_STATUS 0x00000008

#define VERSION "a. base.0001"


#endif // #define BIN_FORMAT_H
