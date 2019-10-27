#include "system_param.h"

static uint32_t g_PWM_LOG   = 0;   // 0=����, 1=�L��
static uint32_t g_UART_LOG  = 0;   // 0=����, 1=�L��
static uint32_t g_SPI_LOG   = 0;   // 0=����, 1=�L��

//--------------------------
// PWM���O�̐ݒ�
uint32_t get_pwm_log_onoff()
{
	return g_PWM_LOG;
}
void     set_pwm_log_onoff( uint32_t val )
{
	g_PWM_LOG = val;
}

//--------------------------
// UART���O�̐ݒ�
uint32_t get_uart_log_onoff()
{
	return g_UART_LOG;
}

void     set_uart_log_onoff( uint32_t val )
{
	g_UART_LOG = val;
}

//--------------------------
// SPI���O�̐ݒ�
uint32_t get_spi_log_onoff()
{
	return g_SPI_LOG;
}

void     set_spi_log_onoff( uint32_t val )
{
	g_SPI_LOG = val;
}

//--------------------------

