/* UART Select Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "pin_assign.h"
#include "driver/can.h"
#include "can.h"
#include "system_param.h"


static can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();
static can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, CAN_MODE_NORMAL);



static void can_sender(void *pvParameters)
{
	static const can_message_t start_message = 
		{	.identifier       = 0x123,
			.data_length_code = 8,
			.flags            = CAN_MSG_FLAG_NONE, 
			.data             = {1, 2 , 3 , 4 ,5 ,6 ,7 ,8}
		};

	while(1)
	{
		can_message_t rx_msg;

		// CANサンプルデータ送信
		can_transmit(&start_message, portMAX_DELAY);
		vTaskDelay(200);

		// CANサンプルデータ受信確認
		can_receive(&rx_msg, portMAX_DELAY);
		if (rx_msg.identifier == 0x123) 
		{
			printf("CAN ID:%x, DLC=%d :", rx_msg.identifier, rx_msg.data_length_code);
			for(int i = 0 ; i < rx_msg.data_length_code ; i++)
			{
				printf("%x ,", rx_msg.data[i]);
			}
			printf("\n");
		}

		vTaskDelay(700);
	}
}


void init_can()
{
	enum CAN_BAUD baud = SystemParam_GetCanBaud();
	
	if( CAN_BOUD_125K == baud )
	{
		can_timing_config_t t_config = CAN_TIMING_CONFIG_125KBITS();
	    ESP_ERROR_CHECK(can_driver_install(&g_config, &t_config, &f_config));
	}
	else if( CAN_BOUD_500K == baud )
	{
		can_timing_config_t t_config = CAN_TIMING_CONFIG_500KBITS();
	    ESP_ERROR_CHECK(can_driver_install(&g_config, &t_config, &f_config));
	}
	else if( CAN_BAUD_1M == baud )
	{
		can_timing_config_t t_config = CAN_TIMING_CONFIG_1MBITS();
	    ESP_ERROR_CHECK(can_driver_install(&g_config, &t_config, &f_config));
	}
}

void send_can()
{
    ESP_ERROR_CHECK(can_start());
	xTaskCreatePinnedToCore(can_sender, "can_sender", 2048, NULL, 12, NULL, 1);
}

