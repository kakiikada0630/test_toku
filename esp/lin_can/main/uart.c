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
#include "uart.h"
#include "system_param.h"
#include "cmd_controller.h"

#define BUF_SIZE (256)

static uint8_t  *data;

static void uart_sender(void *pvParameters)
{
	// Configure a temporary buffer for the incoming data
    for( int i = 0 ; i<BUF_SIZE ; i++ )
    {
		data[i] = i;
	}

	while(1)
	{
		int     length   = 0;
		uint8_t rec[100] = {0};

		// UART送信
		uart_write_bytes(UART_NUM_2, "Uart_Test\n", 10);
		vTaskDelay(200);

		// UART受信確認
		ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_2	, (size_t*)&length));
		if( length > 0 )
		{
			length = uart_read_bytes(UART_NUM_2, rec, length, portMAX_DELAY);
			printf("UART : %s\n", rec);
		}

		vTaskDelay(850);
	}
}

void init_uart()
{
	uint32_t baud = SystemParam_GetUartBaud();
	
    uart_config_t uart_config = {
        .baud_rate = baud,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_2, &uart_config);

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, UART_TX_PIN, UART_RX_PIN, -1, -1));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, BUF_SIZE*2, 0, 0, NULL, 0));
    data = (uint8_t *) malloc(BUF_SIZE);
	uart_set_rx_timeout(UART_NUM_2, 2);

}

void set_uart_baudrate(uint32_t baud)
{
	uart_set_baudrate( UART_NUM_2, baud );
}

void send_uart()
{
	char dummy[1024]  = {0};
	int break_size   = SystemParam_GetUartBreak();
	int senario_size = 0;
	int size = 1024;

	uint32_t baud = SystemParam_GetUartBaud();
	uart_set_baudrate(UART_NUM_2, baud);
	
	// Break送信
	uart_write_bytes_with_break(UART_NUM_2, dummy, 1, break_size);
	
	// シナリオ送信
	senario_size = senario( dummy, size );
	uart_write_bytes(UART_NUM_2, dummy, senario_size);
	
}

uint32_t recv_uart(uint8_t* p_rec, uint32_t size)
{
	int length  = 0;

	ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_2	, (size_t*)&length));
	length = ( length > size )? size : length;

	length = uart_read_bytes(UART_NUM_2, p_rec, length, portMAX_DELAY);
	
	return length;
}
