#include "system_param.h"

static uint32_t       g_UART_BAUD  = 500000;
static uint32_t       g_UART_BREAK = 10000;
static enum CAN_BAUD  g_CAN_BAUD   = CAN_BOUD_125K;


//----------------------------------------
// UART BAUD RATE
//----------------------------------------
void SystemParam_SetUartBaud( uint32_t baud )
{
	g_UART_BAUD = baud;
}

uint32_t SystemParam_GetUartBaud( void )
{
	return g_UART_BAUD;
}

//----------------------------------------
// UART_BREAK SIZE
//----------------------------------------
void     SystemParam_SetUartBreak( uint32_t break_size )
{
	g_UART_BREAK = break_size;
}

uint32_t SystemParam_GetUartBreak( void          )
{
	return g_UART_BREAK;
}

//----------------------------------------
// CAN BAUD RATE
//----------------------------------------
void SystemParam_SetCanBaud( enum CAN_BAUD baud )
{
	g_CAN_BAUD = baud;
}

enum CAN_BAUD SystemParam_GetCanBaud( void )
{
	return g_CAN_BAUD;
}

//---------------------------------------


