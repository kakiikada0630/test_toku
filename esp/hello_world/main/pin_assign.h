#ifndef PIN_ASSIGN_H
#define PIN_ASSIGN_H

#include "freertos/FreeRTOS.h"

//デバッグ用ピン
#define DEBUG_PWM_PIN      GPIO_NUM_25
#define DEBUG_OUT1_PIN     GPIO_NUM_22

//SPIスレーブ用ピン
#define SPI_SLAVE_MISO_PIN -1
#define SPI_SLAVE_MOSI_PIN GPIO_NUM_13
#define SPI_SLAVE_SCLK_PIN GPIO_NUM_14
#define SPI_SLAVE_CS_PIN   GPIO_NUM_15

//SPIマスター用(デバッグ用)ピン
#define SPI_MASTER_MISO_PIN -1
#define SPI_MASTER_MOSI_PIN GPIO_NUM_23
#define SPI_MASTER_SCLK_PIN GPIO_NUM_32
#define SPI_MASTER_CS_PIN   GPIO_NUM_33

//I2C通信
#define I2C_MASTER_SCK GPIO_NUM_5
#define I2C_MASTER_SDA GPIO_NUM_18

//UART用ピン
#define UART_TX_PIN         -1
#define UART_RX_PIN         GPIO_NUM_16

//LED計測用ピン
#define GPIO_A1             GPIO_NUM_2
#define GPIO_A2             GPIO_NUM_4
#define GPIO_B1             GPIO_NUM_34
#define GPIO_B2             GPIO_NUM_39
#define GPIO_B3             GPIO_NUM_36
#define GPIO_DISCHARGE      GPIO_NUM_35

//GPIO出力制御ピン
#define GPIO_TURNSYNC_PIN   GPIO_NUM_12
#define GPIO_HLBKUP_PIN     GPIO_NUM_27
#define GPIO_IG1_PIN        GPIO_NUM_19
#define GPIO_VBU_PIN        GPIO_NUM_21

#endif
