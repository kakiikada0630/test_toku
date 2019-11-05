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
#include "system_param.h"
#include "i2c_server.h"


#define CMD_BUF_SIZE 256

static char  cmd[CMD_BUF_SIZE];


static inline void CMD_HLBackUp( const char* op1 )
{
	if( 0 == memcmp(op1, "on", 2 ) )
	{
		gpio_set_level(GPIO_HLBKUP_PIN, 1);
	}
	else if( 0 == memcmp(op1, "off", 3 ) )
	{
		gpio_set_level(GPIO_HLBKUP_PIN, 0);
	}
	else{}
}

static inline void CMD_TurnSync( const char* op1 )
{
	if( 0 == memcmp(op1, "on", 2 ) )
	{
		gpio_set_level(GPIO_TURNSYNC_PIN, 1);
	}
	else if( 0 == memcmp(op1, "off", 3 ) )
	{
		gpio_set_level(GPIO_TURNSYNC_PIN, 0);
	}
	else{}
}


static inline void CMD_IG1( const char* op1 )
{
	if( 0 == memcmp(op1, "on", 2 ) )
	{
		gpio_set_level(GPIO_IG1_PIN, 1);
	}
	else if( 0 == memcmp(op1, "off", 3 ) )
	{
		gpio_set_level(GPIO_IG1_PIN, 0);
	}
	else{}
}

static inline void CMD_VBU( const char* op1 )
{
	if( 0 == memcmp(op1, "on", 2 ) )
	{
		gpio_set_level(GPIO_VBU_PIN, 1);
	}
	else if( 0 == memcmp(op1, "off", 3 ) )
	{
		gpio_set_level(GPIO_VBU_PIN, 0);
	}
	else{}
}

static inline void CMD_PWM_LOG( const char* op1 )
{
	if( 0 == memcmp(op1, "on", 2 ) )
	{
		set_pwm_log_onoff( 1 );
	}
	else if( 0 == memcmp(op1, "off", 3 ) )
	{
		set_pwm_log_onoff( 0 );
	}
	else{}
}

static inline void CMD_SPI_LOG( const char* op1 )
{
	if( 0 == memcmp(op1, "on", 2 ) )
	{
		set_spi_log_onoff( 1 );
	}
	else if( 0 == memcmp(op1, "off", 3 ) )
	{
		set_spi_log_onoff( 0 );
	}
	else{}
}

static inline void CMD_UART_LOG( const char* op1 )
{
	if( 0 == memcmp(op1, "on", 2 ) )
	{
		set_uart_log_onoff( 1 );
	}
	else if( 0 == memcmp(op1, "off", 3 ) )
	{
		set_uart_log_onoff( 0 );
	}
	else{}
}

static inline void CMD_DacControll( const char* op1, const char* op2 )
{
	uint32_t     ch    = atoi(op1);
	uint32_t     data  = atoi(op2);
	mcp_4728_ch  adc_ch;
	
	if( MCP4728CH_MAX > ch )
	{
		adc_ch = (mcp_4728_ch)ch;
	}
	else
	{
		sprintf( cmd ,"[AdcController]ADC ch Error!! %d\n", ch ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
		return;
	}
	
	if( 0xfff < data )
	{
		sprintf( cmd ,"[AdcController]ADC data invalid!! 0x%x\n", data ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
		return;
	}

	send_i2c_mcp4728( adc_ch, data );
}

static inline void CMD_BINARY_LOG( const char* op1 )
{
	if( 0 == memcmp(op1, "on", 2 ) )
	{
		set_bin_log_onoff( 1 );
	}
	else if( 0 == memcmp(op1, "off", 3 ) )
	{
		set_bin_log_onoff( 0 );
	}
	else{}
}

static void GetCmd(void *pvParameters)
{
	uint32_t index=0;
	
	while(1)
	{
		int data = getc(stdin);
		if( data <= 0 )
		{
			gpio_set_level(GPIO_NUM_22, 0);
			vTaskDelay(1);
			continue;
		}
		gpio_set_level(GPIO_NUM_22, 1);

		cmd[index] = (char)data;
		index++;
		
		if( data == '\n' )
		{
			char cmd_buf[20];
			char option1[10];
			char option2[10];
			
			sscanf( cmd ,"%s %s %s", cmd_buf, option1, option2 );
			//fwrite( cmd, 20, 1, stdout );

			if( 0 == memcmp(cmd_buf, "hlbkup", 6 ) )
			{
				CMD_HLBackUp( option1 );
			}
			else if( 0 == memcmp(cmd_buf, "turn", 4 ) )
			{
				CMD_TurnSync( option1 );
			}
			else if( 0 == memcmp(cmd_buf, "ig1", 3 ) )
			{
				CMD_IG1( option1 );
			}
			else if( 0 == memcmp(cmd_buf, "vbu", 3 ) )
			{
				CMD_VBU( option1 );
			}
			else if( 0 == memcmp(cmd_buf, "pwm", 3 ) )
			{
				CMD_PWM_LOG( option1 );
			}
			else if( 0 == memcmp(cmd_buf, "spi", 3 ) )
			{
				CMD_SPI_LOG( option1 );
			}
			else if( 0 == memcmp(cmd_buf, "uart", 4 ) )
			{
				CMD_UART_LOG( option1 );
			}
			else if( 0 == memcmp(cmd_buf, "dac", 3 ) )
			{
				CMD_DacControll( option1, option2 );
			}
			else if( 0 == memcmp(cmd_buf, "bin", 3 ) )
			{
				CMD_BINARY_LOG( option1 );
			}
			else if( 0 == memcmp(cmd_buf, "echo", 4 ) )
			{
				fwrite( cmd+5, CMD_BUF_SIZE-6, 1, stdout );
			}
			else if( 0 == memcmp(cmd_buf, "help", 4 ) )
			{
				sprintf( cmd ,"*** 電源 ***            \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"ig1    [on/off]         \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"vbu    [on/off]         \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"\n*** 信号線 ***        \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"hlbkup [on/off]         \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"turn   [on/off]         \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"\n*** ログ ***          \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"pwm    [on/off]         \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"spi    [on/off]         \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"uart   [on/off]         \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"dac    [ch] [値(0-4095)]\n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"\n*** その他 ***        \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"echo   [文字列]         \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
			}
			else
			{
				sprintf( cmd ,"#Err Cmd. \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
			}


			memset( cmd, 0, CMD_BUF_SIZE );
			index = 0;
		}
	}
}

void init_cmd()
{
	//TURNSync
	gpio_config_t config1 = {
		.pin_bit_mask = (((uint64_t) 1) << GPIO_TURNSYNC_PIN),
		.mode         = GPIO_MODE_OUTPUT,
		.pull_up_en   = 0,
		.pull_down_en = 0,
		.intr_type    = GPIO_INTR_DISABLE
	};
	gpio_config( &config1 );
	
	//HLBackUp
	gpio_config_t config2 = {
		.pin_bit_mask = (((uint64_t) 1) << GPIO_HLBKUP_PIN),
		.mode         = GPIO_MODE_OUTPUT,
		.pull_up_en   = 0,
		.pull_down_en = 0,
		.intr_type    = GPIO_INTR_DISABLE
	};
	gpio_config( &config2 );
	
	//IG1
	gpio_config_t config3 = {
		.pin_bit_mask = (((uint64_t) 1) << GPIO_IG1_PIN),
		.mode         = GPIO_MODE_OUTPUT,
		.pull_up_en   = 0,
		.pull_down_en = 0,
		.intr_type    = GPIO_INTR_DISABLE
	};
	gpio_config( &config3 );

	//VBU
	gpio_config_t config4 = {
		.pin_bit_mask = (((uint64_t) 1) << GPIO_VBU_PIN),
		.mode         = GPIO_MODE_OUTPUT,
		.pull_up_en   = 0,
		.pull_down_en = 0,
		.intr_type    = GPIO_INTR_DISABLE
	};
	gpio_config( &config4 );
	
	//I2Cドライバ初期化 (ADC制御用)
	init_i2c();

	xTaskCreatePinnedToCore(GetCmd  , "cmd_controller", 1028, NULL, 12, NULL, 0);
}
