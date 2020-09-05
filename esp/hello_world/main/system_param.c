#include "system_param.h"

static uint32_t g_PWM_LOG   = 0;   // 0=����, 1=�L��
static uint32_t g_UART_LOG  = 0;   // 0=����, 1=�L��
static uint32_t g_LIN_LOG   = 0;   // 0=����, 1=�L��
static uint32_t g_SPI_LOG   = 0;   // 0=����, 1=�L��
static uint32_t g_BIN_LOG   = 0;   // 0=����, 1=�L��

static uint32_t ADC_LCM     = 3000;// LCM���x  0.01�� LSB
static uint32_t ADC_LED1    = 3000;// LED1���x 0.01�� LSB
static uint32_t ADC_LED2    = 3000;// LED2���x 0.01�� LSB
static uint32_t ADC_LED3    = 3000;// LED3���x 0.01�� LSB


//--------------------------
// PWM���O�̐ݒ�
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
// UART���O�̐ݒ�
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
// LIN���O�̐ݒ�
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
// SPI���O�̐ݒ�
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
// BIN���O�̐ݒ�
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
// LCM���x
uint32_t get_lcm_dec()
{
	return ADC_LCM;
}

void     set_lcm_dec( uint32_t val )
{
	ADC_LCM  = val;
}

//--------------------------
// LED1���x
uint32_t get_led1_dec()
{
	return ADC_LED1;
}

void     set_led1_dec( uint32_t val )
{
	ADC_LED1  = val;
}

//--------------------------
// LED2���x
uint32_t get_led2_dec()
{
	return ADC_LED2;
}

void     set_led2_dec( uint32_t val )
{
	ADC_LED2  = val;
}

//--------------------------
// LED3���x
uint32_t get_led3_dec()
{
	return ADC_LED3;
}

void     set_led3_dec( uint32_t val )
{
	ADC_LED3  = val;
}

//--------------------------


