/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cmd_controller.h"
#include "pin_assign.h"
#include "uart.h"
#include "system_param.h"

#define CMD_BUF_SIZE 256
static char cmd[CMD_BUF_SIZE];
static char g_senario[1024];
static int g_senario_size=0;

void f_cmd_program()
{
	char      l_str_hex[2]   = {0,0};
	uint32_t  l_index        = 0;

	sprintf( cmd ,"please input senario     \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
	memset( g_senario, 0, 1024 );
	memset( cmd, 0, CMD_BUF_SIZE );
	g_senario_size = 0;

	while(1)
	{
		int l_data = getc(stdin);
		if( l_data <= 0 )
		{
			vTaskDelay(1);
			continue;
		}

		l_str_hex[l_index] = (char)l_data;
		l_index = (l_index + 1)%2;
		
		if( l_data == '\n' )
		{
			sprintf( cmd ,"                         \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
			sprintf( cmd ,"senario finish!          \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
			break;
		}
		
		if( 0 == l_index)
		{
			g_senario[g_senario_size] = strtol( l_str_hex, NULL, 16);
			g_senario_size++;
			
			//sprintf( cmd ,"                         \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
			//sprintf( cmd ,"data:%x     \n", l_data); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
			
		}
	}
}




enum CMD_ST exec_cmd()
{
	static uint32_t   l_index=0;
	enum CMD_ST       l_stat =CMD_ST_IDLE;

	while(1)
	{
		int l_data = getc(stdin);
		if( l_data <= 0 )
		{
			vTaskDelay(1);
			break;
		}

		cmd[l_index] = (char)l_data;
		l_index++;
		
		if( l_data == '\n' )
		{
			char cmd_buf[20]={0};
			char option1[10]={0};
			char option2[10]={0};
			
			sscanf( cmd ,"%s %s %s", cmd_buf, option1, option2 );

			// CAN設定コマンド
			if( 0 == memcmp(cmd_buf, "can", 3 ) )
			{
				// CANのボーレート
				if( 0 == memcmp(option1, "baud", 4 ) )
				{
					if( 0 == memcmp(option2, "125", 3 ) )
					{
						SystemParam_SetCanBaud(CAN_BOUD_125K);
					}
					else if( 0 == memcmp(option2, "500", 3 ) )
					{
						SystemParam_SetCanBaud(CAN_BOUD_500K);
					}
					else if( 0 == memcmp(option2, "1m", 2 ) )
					{
						SystemParam_SetCanBaud(CAN_BAUD_1M);
					}
					else
					{
						sprintf( cmd ,"iligal command          \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
					}
				}
				else
				{
					sprintf( cmd ,"iligal command          \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				}
			}
			// UART設定コマンド
			else if( 0 == memcmp(cmd_buf, "uart", 4 ) )
			{
				// UARTのボーレート
				if( 0 == memcmp(option1, "baud", 4 ) )
				{
					int l_baud = atoi(option2);
					
					if( (9800 <= l_baud) && (l_baud <= 1500000) )
					{
						SystemParam_SetUartBaud(l_baud);
					}
					else
					{
						sprintf( cmd ,"iligal command          \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
					}
				}
				else if( 0 == memcmp(option1, "break", 5 ) )
				{
					int l_break = atoi(option2);

					if( (10 <= l_break) || (l_break == 0) )
					{
						SystemParam_SetUartBreak(l_break);
					}
					else
					{
						sprintf( cmd ,"iligal command          \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
					}
				}
				else
				{
					sprintf( cmd ,"iligal command          \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				}
			}
			// UART設定コマンド
			else if( 0 == memcmp(cmd_buf, "senario", 7 ) )
			{
				f_cmd_program();
			}
			// UART設定コマンド
			else if( 0 == memcmp(cmd_buf, "start", 7 ) )
			{
				sprintf( cmd ,"senario start!                                      \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				l_stat = CMD_ST_EXEC;
			}
			else if( 0 == memcmp(cmd_buf, "help", 4 ) )
			{
				sprintf( cmd ,"*** UART コマンド ***                               \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"uart [baud/break] [val]                             \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ," - baud ： val= baud rate.       9800<val<1500000   \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ," - break： val= break bit size.  val>10 or val=0    \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"*** CAN コマンド ***                                \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"can [baud] [val]                                    \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ," - baud ： val= baud rate.    val=125k or 500k or 1m\n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"*** Senario コマンド ***                            \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"senario                                             \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"start                                               \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
			}
			else
			{
				//sprintf( cmd ,"#Err Cmd. \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
			}

			memset( cmd, 0, CMD_BUF_SIZE );
			l_index = 0;
		}
	}

	return l_stat;
}

void init_cmd()
{
//	//TURNSync
//	gpio_config_t config1 = {
//		.pin_bit_mask = (((uint64_t) 1) << GPIO_TURNSYNC_PIN),
//		.mode         = GPIO_MODE_OUTPUT,
//		.pull_up_en   = 0,
//		.pull_down_en = 0,
//		.intr_type    = GPIO_INTR_DISABLE
//	};
//	gpio_config( &config1 );
//	
//	//HLBackUp
//	gpio_config_t config2 = {
//		.pin_bit_mask = (((uint64_t) 1) << GPIO_HLBKUP_PIN),
//		.mode         = GPIO_MODE_OUTPUT,
//		.pull_up_en   = 0,
//		.pull_down_en = 0,
//		.intr_type    = GPIO_INTR_DISABLE
//	};
//	gpio_config( &config2 );
//	
//	//IG1
//	gpio_config_t config3 = {
//		.pin_bit_mask = (((uint64_t) 1) << GPIO_IG1_PIN),
//		.mode         = GPIO_MODE_OUTPUT,
//		.pull_up_en   = 0,
//		.pull_down_en = 0,
//		.intr_type    = GPIO_INTR_DISABLE
//	};
//	gpio_config( &config3 );
//
//	//VBU
//	gpio_config_t config4 = {
//		.pin_bit_mask = (((uint64_t) 1) << GPIO_VBU_PIN),
//		.mode         = GPIO_MODE_OUTPUT,
//		.pull_up_en   = 0,
//		.pull_down_en = 0,
//		.intr_type    = GPIO_INTR_DISABLE
//	};
//	gpio_config( &config4 );
//	
//	//I2Cドライバ初期化 (ADC制御用)
//	init_i2c();
//
//	CMD_DacControll( "0", "30" );
//	CMD_DacControll( "1", "30" );
//	CMD_DacControll( "2", "30" );
//	CMD_DacControll( "3", "30" );
//	//xTaskCreatePinnedToCore(GetCmd  , "cmd_controller", 1028, NULL, 12, NULL, 0);
}

int senario( char *data , int size )
{
	memcpy( data, g_senario, g_senario_size );
	
	return g_senario_size;
}

