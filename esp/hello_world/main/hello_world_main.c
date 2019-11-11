/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/ledc.h"
#include "gpio_discharge.h"
#include "gpio_A1.h"
#include "gpio_A2.h"
#include "gpio_B1.h"
#include "gpio_B2.h"
#include "gpio_B3.h"
#include "gpio_UDIM21.h"
#include "gpio_UDIM22.h"
#include "spi_slave_peri.h"
#include "spi_master.h"
#include "uart_lmm.h"
#include "cmd_controller.h"
#include "pin_assign.h"
#include "system_param.h"


#define SPI_DATA_SIZE 100  //uartデータサイズ
#define URT_DATA_SIZE 100  //spiデータサイズ

void set_pwm()
{
    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT,    // resolution of PWM duty
        .freq_hz         = 200,                  // frequency of PWM signal
        .speed_mode      = LEDC_HIGH_SPEED_MODE, // timer mode
        .timer_num       = LEDC_TIMER_0          // timer index
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_CHANNEL_0,
        .duty       = 716,
        .gpio_num   = DEBUG_PWM_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER_0
    };

    // Set LED Controller with previously prepared configuration
    ledc_channel_config(&ledc_channel);
}

//static void SendData()
static void SendData(void *pvParameters)
{
    int32_t         i              = 0;
    static int64_t  start_time     = 0;
    static int64_t  spi_latest_time= 0;
	static uint8_t  urt_data[URT_DATA_SIZE] = {0};
	static uint16_t spi_data[SPI_DATA_SIZE] = {0};
	int32_t spi_size = 0;
	int32_t urt_size = 0;

	while (1) {
		memset( urt_data, 0, sizeof(uint8_t )*URT_DATA_SIZE );
		memset( spi_data, 0, sizeof(uint16_t)*SPI_DATA_SIZE );

		spi_size        = get_spi_data( spi_data, SPI_DATA_SIZE );
		urt_size        = recv_uart   ( urt_data, URT_DATA_SIZE );
		spi_latest_time = get_spi_ratest_time();

		int32_t  buf;
		uint32_t pwm_log  = get_pwm_log_onoff();
		uint32_t spi_log  = get_spi_log_onoff();
		uint32_t uart_log = get_uart_log_onoff();
		uint32_t bin_log  = get_bin_log_onoff();
		start_time        = esp_timer_get_time();

		if( pwm_log|spi_log|uart_log ) 
		{
			buf = (int32_t) (start_time/1000);
			//ログ出力する設定なら、先頭にチックを出す
			printf("[%6d]",buf);
		}

		if( pwm_log )
		{
	        printf("%4d.%2d%4d.%2d%4d.%2d%4d.%2d%4d.%2d%4d.%2d%4d.%2d%4d.%2d ",
	                  get_percent_discharge()/100,get_percent_discharge()%100,
	                  get_percent_A1()       /100,get_percent_A1()       %100,
	                  get_percent_A2()       /100,get_percent_A2()       %100,
	                  get_percent_B1()       /100,get_percent_B1()       %100,
	                  get_percent_B2()       /100,get_percent_B2()       %100,
	                  get_percent_B3()       /100,get_percent_B3()       %100,
	                  get_percent_UDIM21()   /100,get_percent_UDIM21()   %100,
	                  get_percent_UDIM22()   /100,get_percent_UDIM22()   %100
	                  );
	    }

		if( spi_log )
		{
			uint8_t *buf         = (uint8_t *)spi_data;
			uint8_t buf_spi_size = spi_size*2;
			for(uint32_t j=0 ; j<buf_spi_size ; j++)
			{
				printf( "%02x", buf[j] );
			}
		}

		if( uart_log )
		{
			for(uint32_t j=0 ; j<urt_size ; j++)
			{
				printf( "%02x", urt_data[j] );
			}
		}

		if( pwm_log|spi_log|uart_log ) 
		{
			printf("\n");
		}

		if( bin_log )
		{
			unsigned char bin_buf[250];  //0〜7:スタート符号 8〜11:チック  12〜39:PWM  40〜139:SPI  140〜239:UART
			uint32_t *tick_pnt = 0;
			uint16_t *buf_pnt  = 0;
			uint8_t  *spi_pnt  = 0;
			uint8_t  *urt_pnt  = 0;
			uint16_t *adc_pnt  = 0;
			
			tick_pnt = (uint32_t *)&bin_buf[  8];
			buf_pnt  = (uint16_t *)&bin_buf[ 12];
			spi_pnt  = (uint8_t  *)&bin_buf[ 40];
			urt_pnt  = (uint8_t  *)&bin_buf[140];
			adc_pnt  = (uint16_t *)&bin_buf[240];
			
			memset(bin_buf, 0, 250 );
			
			//スタート符号
			for(uint32_t j=0 ; j < 8 ; j++ )
			{
				bin_buf[j]=j+0xE0;
			}
			
			//チック
			*tick_pnt = (int32_t) (start_time/1000);

			//PWM
			*(buf_pnt+0)=(uint16_t)get_percent_discharge();
			*(buf_pnt+1)=(uint16_t)get_percent_A1();
			*(buf_pnt+2)=(uint16_t)get_percent_A2();
			*(buf_pnt+3)=(uint16_t)get_percent_B1();
			*(buf_pnt+4)=(uint16_t)get_percent_B2();
			*(buf_pnt+5)=(uint16_t)get_percent_B3();
			*(buf_pnt+6)=(uint16_t)get_percent_UDIM21();
			*(buf_pnt+7)=(uint16_t)get_percent_UDIM22();

			//SPI
			uint8_t *spi_data_buf = (uint8_t *)spi_data;
			for(uint32_t j=0 ; j<spi_size*2 ; j++)
			{
				*(spi_pnt+j) = *(spi_data_buf+j);
			}
			
			//UART
			for(uint32_t j=0 ; j<urt_size ; j++)
			{
				*(urt_pnt+j) = *(urt_data+j);
			}
			
			*(adc_pnt  ) = get_lcm_dec();
			*(adc_pnt+1) = get_led1_dec();
			*(adc_pnt+2) = get_led2_dec();
			*(adc_pnt+3) = get_led3_dec();
			
			fwrite(bin_buf, 250, 1, stdout);
		}

        i++;

		while(1)
		{
			int64_t base_time;
			int64_t check_time = esp_timer_get_time();

			// spiを受信している場合、最後にspiを受信した時間をベースに
			// 次のループまでのwait時間を調整する。spiが5ms以上受信していない
			// ない場合、ループの開始時間をベースに、次のループまでのwait時間を調整する。
			base_time = ( check_time - spi_latest_time < (int64_t)5000 )? spi_latest_time : start_time;

			if( base_time+(int64_t)4600 < check_time )
			{
				break;
			}
            //ets_delay_us(50);
			vTaskDelay(1);
		}

    }
}

void app_main()
{
	init_uart();

	//------------------------
	// デバッグ用初期化処理
	//------------------------
    //init_spi_master();
    //set_pwm();
    //send_uart();
	//------------------------

    gpio_install_isr_service(0);
    init_gpio_discharge();
    init_gpio_A1();
    init_gpio_A2();
    init_gpio_B1();
    init_gpio_B2();
    init_gpio_B3();
    init_gpio_UDIM21();
    init_gpio_UDIM22();

	init_cmd();
    init_spi_slave();

	xTaskCreatePinnedToCore(SendData, "main_loop", 4096, NULL, 12, NULL, 0);

    while (1) 
    {
		vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
