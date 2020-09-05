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
// LINログの設定
uint32_t get_lin_log_onoff();
void     set_lin_log_onoff( uint32_t val );

//--------------------------
// SPIログの設定
uint32_t get_spi_log_onoff();
void     set_spi_log_onoff( uint32_t val );

//--------------------------
// BINログの設定
uint32_t get_bin_log_onoff();
void     set_bin_log_onoff( uint32_t val );

//--------------------------
// LCM温度
uint32_t get_lcm_dec();
void     set_lcm_dec( uint32_t val );

//--------------------------
// LED1温度
uint32_t get_led1_dec();
void     set_led1_dec( uint32_t val );

//--------------------------
// LED2温度
uint32_t get_led2_dec();
void     set_led2_dec( uint32_t val );

//--------------------------
// LED3温度
uint32_t get_led3_dec();
void     set_led3_dec( uint32_t val );

//--------------------------


#endif
