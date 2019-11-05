#ifndef SYSTEM_PARAM_H
#define SYSTEM_PARAM_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"

//--------------------------
// PWMログの設定
uint32_t get_pwm_log_onoff();
void     set_pwm_log_onoff( uint32_t val );

//--------------------------
// UARTログの設定
uint32_t get_uart_log_onoff();
void     set_uart_log_onoff( uint32_t val );

//--------------------------
// SPIログの設定
uint32_t get_spi_log_onoff();
void     set_spi_log_onoff( uint32_t val );

//--------------------------
// BINログの設定
uint32_t get_bin_log_onoff();
void     set_bin_log_onoff( uint32_t val );

#endif
