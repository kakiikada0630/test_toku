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
#include "uart_lin.h"

#define BUF_SIZE (200)

void init_lin()
{
    uart_config_t uart_config = {
        .baud_rate = 20000,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, LIN_TX_PIN, LIN_RX_PIN, -1, -1));

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, BUF_SIZE, 0, 0, NULL, 0));
}

void set_lin_baudrate(uint32_t baud)
{
	uart_set_baudrate( UART_NUM_1, baud );
}

uint32_t recv_lin(uint8_t* p_rec, uint32_t size)
{
	int length  = 0;
	int length2 = 0;

	// 150us待ってもサイズが変化しなかった場合、一連の
	// 命令列を受信完了と判断する。
	do{
		ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_1	, (size_t*)&length));
		ets_delay_us(150);
		//vTaskDelay(1);
		ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_1	, (size_t*)&length2));
	}while( length != length2 );

	length = ( length > size )? size : length;

	length = uart_read_bytes(UART_NUM_1, p_rec, length, 0);
	
	return length;
}

