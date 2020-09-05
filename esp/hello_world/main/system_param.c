#include "system_param.h"

static uint32_t g_PWM_LOG   = 0;   // 0=無効, 1=有効
static uint32_t g_UART_LOG  = 0;   // 0=無効, 1=有効
static uint32_t g_LIN_LOG   = 0;   // 0=無効, 1=有効
static uint32_t g_SPI_LOG   = 0;   // 0=無効, 1=有効
static uint32_t g_BIN_LOG   = 0;   // 0=無効, 1=有効

static uint32_t ADC_LCM     = 3000;// LCM温度  0.01℃ LSB
static uint32_t ADC_LED1    = 3000;// LED1温度 0.01℃ LSB
static uint32_t ADC_LED2    = 3000;// LED2温度 0.01℃ LSB
static uint32_t ADC_LED3    = 3000;// LED3温度 0.01℃ LSB


//--------------------------
// PWMログの設定
uint32_t get_pwm_log_onoff()
{
	return g_PWM_LOG;
}
void     set_pwm_log_onoff( uint32_t val )
{
	if(1 == val)
	{
		g_BIN_LOG = 0;
	}
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
	if(1 == val)
	{
		g_BIN_LOG = 0;
	}
	g_UART_LOG = val;
}

//--------------------------
// LINログの設定
uint32_t get_lin_log_onoff()
{
	return g_LIN_LOG;
}

void     set_lin_log_onoff( uint32_t val )
{
	if(1 == val)
	{
		g_BIN_LOG = 0;
	}
	g_LIN_LOG = val;
}

//--------------------------
// SPIログの設定
uint32_t get_spi_log_onoff()
{
	return g_SPI_LOG;
}

void     set_spi_log_onoff( uint32_t val )
{
	if(1 == val)
	{
		g_BIN_LOG = 0;
	}

	g_SPI_LOG = val;
}

//--------------------------
// BINログの設定
uint32_t get_bin_log_onoff()
{
	return g_BIN_LOG;
}


void     set_bin_log_onoff( uint32_t val )
{
	if(1 == val)
	{
		g_PWM_LOG  = 0;
		g_UART_LOG = 0;
		g_SPI_LOG  = 0;
	}
	g_BIN_LOG  = val;
}


//--------------------------
// LCM温度
uint32_t get_lcm_dec()
{
	return ADC_LCM;
}

void     set_lcm_dec( uint32_t val )
{
	ADC_LCM  = val;
}

//--------------------------
// LED1温度
uint32_t get_led1_dec()
{
	return ADC_LED1;
}

void     set_led1_dec( uint32_t val )
{
	ADC_LED1  = val;
}

//--------------------------
// LED2温度
uint32_t get_led2_dec()
{
	return ADC_LED2;
}

void     set_led2_dec( uint32_t val )
{
	ADC_LED2  = val;
}

//--------------------------
// LED3温度
uint32_t get_led3_dec()
{
	return ADC_LED3;
}

void     set_led3_dec( uint32_t val )
{
	ADC_LED3  = val;
}

//--------------------------


