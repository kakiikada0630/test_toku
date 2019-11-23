/* UART Select Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "pin_assign.h"

#define BUF_SIZE (200)

static uint8_t  *data;


static void uart_sender(void *pvParameters)
{
	static uint64_t base = 0;
	static uint64_t cur  = 0;

    // Configure a temporary buffer for the incoming data
    for( int i = 0 ; i<BUF_SIZE ; i++ )
    {
		data[i] = i;
	}

	while(1)
	{
		base = esp_timer_get_time();

	    uart_write_bytes(UART_NUM_2, (const char*)data, 50);
		
		while(1)
		{
			cur = esp_timer_get_time();

			if(base+5000 < cur)
			{
				break;
			}

			vTaskDelay(1);
		}
	}
}

void init_uart()
{
    uart_config_t uart_config = {
        .baud_rate = 500000,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_2, &uart_config);

    // Set UART pins(TX: IO16 (UART2 default), RX: IO17 (UART2 default), RTS: IO18, CTS: IO19)
    //ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 17, 16, -1, -1));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, UART_TX_PIN, UART_RX_PIN, -1, -1));

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, BUF_SIZE*2, 0, 0, NULL, 0));
    data = (uint8_t *) malloc(BUF_SIZE);
}

void send_uart()
{
	xTaskCreatePinnedToCore(uart_sender, "uart_sender", 2048, NULL, 12, NULL, 1);
}

uint32_t recv_uart(uint8_t* p_rec, uint32_t size)
{
	int length  = 0;
	int length2 = 0;

	// 200us待ってもサイズが変化しなかった場合、一連の
	// 命令列を受信完了と判断する。
	do{
		ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_2	, (size_t*)&length));
		ets_delay_us(200);
		//vTaskDelay(1);
		ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_2	, (size_t*)&length2));
	}while( length != length2 );

	length = ( length > size )? size : length;

	length = uart_read_bytes(UART_NUM_2, p_rec, length, portMAX_DELAY);
	
	return length;
}

