#ifndef SYSTEM_PARAM_H
#define SYSTEM_PARAM_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"

//--------------------------
// PWM���O�̐ݒ�
uint32_t get_pwm_log_onoff();
void     set_pwm_log_onoff( uint32_t val );

//--------------------------
// UART���O�̐ݒ�
uint32_t get_uart_log_onoff();
void     set_uart_log_onoff( uint32_t val );

//--------------------------
// SPI���O�̐ݒ�
uint32_t get_spi_log_onoff();
void     set_spi_log_onoff( uint32_t val );

#endif
