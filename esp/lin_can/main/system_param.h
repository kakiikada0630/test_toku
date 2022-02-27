#ifndef SYSTEM_PARAM_H
#define SYSTEM_PARAM_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"

enum CAN_BAUD
{
	CAN_BOUD_125K,
	CAN_BOUD_500K,
	CAN_BAUD_1M
};


//--------------------------
// UART_BAUDê›íË
void     SystemParam_SetUartBaud( uint32_t baud );
uint32_t SystemParam_GetUartBaud( void          );

//--------------------------
// UART_BREAKê›íË
void     SystemParam_SetUartBreak( uint32_t break_size );
uint32_t SystemParam_GetUartBreak( void                );

//--------------------------
// CAN_BAUDê›íË
void          SystemParam_SetCanBaud( enum CAN_BAUD baud );
enum CAN_BAUD SystemParam_GetCanBaud( void               );



#endif
