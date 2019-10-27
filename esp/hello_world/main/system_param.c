#include "system_param.h"

static uint32_t g_PWM_LOG   = 0;   // 0=無効, 1=有効
static uint32_t g_UART_LOG  = 0;   // 0=無効, 1=有効
static uint32_t g_SPI_LOG   = 0;   // 0=無効, 1=有効

//--------------------------
// PWMログの設定
uint32_t get_pwm_log_onoff()
{
	return g_PWM_LOG;
}
void     set_pwm_log_onoff( uint32_t val )
{
	g_PWM_LOG = val;
}

//--------------------------
// UARTログの設定
uint32_t get_uart_log_onoff()
{
	return g_UART_LOG;
}

void     set_uart_log_onoff( uint32_t val )
{
	g_UART_LOG = val;
}

//--------------------------
// SPIログの設定
uint32_t get_spi_log_onoff()
{
	return g_SPI_LOG;
}

void     set_spi_log_onoff( uint32_t val )
{
	g_SPI_LOG = val;
}

//--------------------------

