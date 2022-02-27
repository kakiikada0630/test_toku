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
#include "lin.h"

#define BUF_SIZE (256)


static void lin_sender(void *pvParameters)
{
	uint8_t data[BUF_SIZE];
	
    // Configure a temporary buffer for the incoming data
    for( int i = 0 ; i<BUF_SIZE ; i++ )
    {
		data[i] = i;
	}

	while(1)
	{
	    uart_write_bytes(UART_NUM_1, "lin_Test\n", 9);
		
		vTaskDelay(1000);
	}
}


void init_lin()
{
    uart_config_t uart_config = {
        .baud_rate = 100000,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, LIN_TX_PIN, LIN_RX_PIN, -1, -1));

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, BUF_SIZE, 0, 0, NULL, 0));
	uart_set_rx_timeout(UART_NUM_1, 2);
}

void set_lin_baudrate(uint32_t baud)
{
	uart_set_baudrate( UART_NUM_1, baud );
}

void send_lin()
{
	xTaskCreatePinnedToCore(lin_sender, "lin_sender", 2048, NULL, 12, NULL, 1);
}

uint32_t recv_lin(uint8_t* p_rec, uint32_t size)
{
	int length  = 0;

	// 150us�҂��Ă��T�C�Y���ω����Ȃ������ꍇ�A��A��
	// ���ߗ����M�����Ɣ��f����B
	ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_1	, (size_t*)&length));

	length = ( length > size )? size : length;

	length = uart_read_bytes(UART_NUM_1, p_rec, length, 0);
	
	return length;
}

