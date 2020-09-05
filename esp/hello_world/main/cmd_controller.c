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
#include "uart_lmm.h"
#include "uart_lin.h"


#define DEC2DAC_TABLE_SIZE  195

struct DEC2DAC
{
	int decimal;
	int dac;
} dec2dac[] = {
	{-40	,	3958},
	{-39	,	3951},
	{-38	,	3943},
	{-37	,	3935},
	{-36	,	3927},
	{-35	,	3918},
	{-34	,	3909},
	{-33	,	3900},
	{-32	,	3890},
	{-31	,	3880},
	{-30	,	3869},
	{-29	,	3857},
	{-28	,	3846},
	{-27	,	3834},
	{-26	,	3821},
	{-25	,	3808},
	{-24	,	3794},
	{-23	,	3780},
	{-22	,	3765},
	{-21	,	3749},
	{-20	,	3734},
	{-19	,	3717},
	{-18	,	3700},
	{-17	,	3683},
	{-16	,	3664},
	{-15	,	3645},
	{-14	,	3627},
	{-13	,	3606},
	{-12	,	3586},
	{-11	,	3564},
	{-10	,	3542},
	{ -9	,	3520},
	{ -8	,	3496},
	{ -7	,	3473},
	{ -6	,	3449},
	{ -5	,	3423},
	{ -4	,	3398},
	{ -3	,	3372},
	{ -2	,	3345},
	{ -1	,	3317},
	{  0	,	3289},
	{  1	,	3260},
	{  2	,	3231},
	{  3	,	3201},
	{  4	,	3170},
	{  5	,	3139},
	{  6	,	3108},
	{  7	,	3076},
	{  8	,	3043},
	{  9	,	3011},
	{ 10	,	2977},
	{ 11	,	2943},
	{ 12	,	2909},
	{ 13	,	2874},
	{ 14	,	2839},
	{ 15	,	2803},
	{ 16	,	2768},
	{ 17	,	2732},
	{ 18	,	2696},
	{ 19	,	2659},
	{ 20	,	2623},
	{ 21	,	2586},
	{ 22	,	2549},
	{ 23	,	2512},
	{ 24	,	2475},
	{ 25	,	2437},
	{ 26	,	2400},
	{ 27	,	2363},
	{ 28	,	2326},
	{ 29	,	2288},
	{ 30	,	2251},
	{ 31	,	2214},
	{ 32	,	2177},
	{ 33	,	2140},
	{ 34	,	2103},
	{ 35	,	2067},
	{ 36	,	2030},
	{ 37	,	1994},
	{ 38	,	1959},
	{ 39	,	1923},
	{ 40	,	1888},
	{ 41	,	1853},
	{ 42	,	1818},
	{ 43	,	1784},
	{ 44	,	1750},
	{ 45	,	1717},
	{ 46	,	1683},
	{ 47	,	1650},
	{ 48	,	1618},
	{ 49	,	1586},
	{ 50	,	1554},
	{ 51	,	1523},
	{ 52	,	1492},
	{ 53	,	1462},
	{ 54	,	1432},
	{ 55	,	1403},
	{ 56	,	1374},
	{ 57	,	1346},
	{ 58	,	1318},
	{ 59	,	1291},
	{ 60	,	1264},
	{ 61	,	1237},
	{ 62	,	1210},
	{ 63	,	1185},
	{ 64	,	1160},
	{ 65	,	1135},
	{ 66	,	1111},
	{ 67	,	1088},
	{ 68	,	1064},
	{ 69	,	1041},
	{ 70	,	1019},
	{ 71	,	 997},
	{ 72	,	 975},
	{ 73	,	 954},
	{ 74	,	 934},
	{ 75	,	 913},
	{ 76	,	 894},
	{ 77	,	 874},
	{ 78	,	 855},
	{ 79	,	 836},
	{ 80	,	 818},
	{ 81	,	 801},
	{ 82	,	 783},
	{ 83	,	 767},
	{ 84	,	 749},
	{ 85	,	 733},
	{ 86	,	 717},
	{ 87	,	 702},
	{ 88	,	 687},
	{ 89	,	 672},
	{ 90	,	 658},
	{ 91	,	 643},
	{ 92	,	 629},
	{ 93	,	 616},
	{ 94	,	 603},
	{ 95	,	 590},
	{ 96	,	 577},
	{ 97	,	 564},
	{ 98	,	 553},
	{ 99	,	 541},
	{100	,	 529},
	{101	,	 518},
	{102	,	 507},
	{103	,	 496},
	{104	,	 486},
	{105	,	 475},
	{106	,	 465},
	{107	,	 455},
	{108	,	 446},
	{109	,	 437},
	{110	,	 428},
	{111	,	 419},
	{112	,	 410},
	{113	,	 401},
	{114	,	 393},
	{115	,	 385},
	{116	,	 377},
	{117	,	 369},
	{118	,	 362},
	{119	,	 355},
	{120	,	 347},
	{121	,	 340},
	{122	,	 333},
	{123	,	 327},
	{124	,	 320},
	{125	,	 314},
	{126	,	 307},
	{127	,	 301},
	{128	,	 295},
	{129	,	 289},
	{130	,	 283},
	{131	,	 278},
	{132	,	 273},
	{133	,	 268},
	{134	,	 263},
	{135	,	 258},
	{136	,	 253},
	{137	,	 248},
	{138	,	 243},
	{139	,	 238},
	{140	,	 234},
	{141	,	 230},
	{142	,	 226},
	{143	,	 222},
	{144	,	 218},
	{145	,	 214},
	{146	,	 210},
	{147	,	 206},
	{148	,	 202},
	{149	,	 199},
	{150	,	 196},
	{151	,	 192},
	{152	,	 189},
	{153	,	 186},
	{154	,	 183}
};



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

static inline void CMD_UART_LOG( const char* op1, const char* op2 )
{
	if( 0 == memcmp(op1, "on", 2 ) )
	{
		set_uart_log_onoff( 1 );
	}
	else if( 0 == memcmp(op1, "off", 3 ) )
	{
		set_uart_log_onoff( 0 );
	}
	else if( 0 == memcmp(op1, "baud", 4 ) )
	{
		uint32_t boud_rate = atoi(op2);
		set_uart_baudrate( boud_rate );
	}
	else{}
}

static inline void CMD_LIN_LOG( const char* op1, const char* op2 )
{
	if( 0 == memcmp(op1, "on", 2 ) )
	{
		set_lin_log_onoff( 1 );
	}
	else if( 0 == memcmp(op1, "off", 3 ) )
	{
		set_lin_log_onoff( 0 );
	}
	else if( 0 == memcmp(op1, "baud", 4 ) )
	{
		uint32_t boud_rate = atoi(op2);
		set_lin_baudrate( boud_rate );
	}
	else{}
}

void CMD_ConvertDec2ADC( int dec, int* dac )
{
	int index = dec+40;  // tableのindex=0がdec=-40のため。
	
	if     ( index < 0 )
	{
		*dac = dec2dac[0].dac;
	}
	else if( index >= DEC2DAC_TABLE_SIZE )
	{
		*dac = dec2dac[DEC2DAC_TABLE_SIZE-1].dac;
	}
	else
	{
		*dac = dec2dac[index].dac;
	}
}

static inline void CMD_DacControll( const char* op1, const char* op2 )
{
	uint32_t     ch    = atoi(op1);
	int          data  = atoi(op2);
	int          dac   = 0;
	mcp_4728_ch  dac_ch;


	CMD_ConvertDec2ADC( data, &dac );
	
	if( MCP4728CH_MAX > ch )
	{
		dac_ch = (mcp_4728_ch)ch;
	}
	else
	{
		//sprintf( cmd ,"[AdcController]DAC ch Error!! %d\n", ch ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
		return;
	}
	
	if( 0xfff < data )
	{
		//sprintf( cmd ,"[AdcController]ADC data invalid!! 0x%x\n", data ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
		return;
	}

	send_i2c_mcp4728( dac_ch, dac );
	
	switch(dac_ch){
		case MCP4728CH_0:{ set_lcm_dec (data); break; }
		case MCP4728CH_1:{ set_led1_dec(data); break; }
		case MCP4728CH_2:{ set_led2_dec(data); break; }
		case MCP4728CH_3:{ set_led3_dec(data); break; }
		default:{}
	}
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

void exec_cmd()
{
	static uint32_t index=0;

	while(1)
	{
		int data = getc(stdin);
		if( data <= 0 )
		{
			vTaskDelay(1);
			break;
		}

		cmd[index] = (char)data;
		index++;
		
		if( data == '\n' )
		{
			char cmd_buf[20]={0};
			char option1[10]={0};
			char option2[10]={0};
			
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
				CMD_UART_LOG( option1, option2 );
			}
			else if( 0 == memcmp(cmd_buf, "lin", 4 ) )
			{
				CMD_LIN_LOG( option1, option2 );
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
				//fwrite( cmd+5, CMD_BUF_SIZE-6, 1, stdout );
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
				sprintf( cmd ,"uart   [on/off/baud] [rate] \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"lin    [on/off/baud] [rate] \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"dac    [ch] [値(0-4095)]\n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"\n*** その他 ***        \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
				sprintf( cmd ,"echo   [文字列]         \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
			}
			else
			{
				//sprintf( cmd ,"#Err Cmd. \n" ); fwrite( cmd, CMD_BUF_SIZE, 1, stdout );
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

	CMD_DacControll( "0", "30" );
	CMD_DacControll( "1", "30" );
	CMD_DacControll( "2", "30" );
	CMD_DacControll( "3", "30" );
	//xTaskCreatePinnedToCore(GetCmd  , "cmd_controller", 1028, NULL, 12, NULL, 0);
}
