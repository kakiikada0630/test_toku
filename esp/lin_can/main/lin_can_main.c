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
#include "lin.h"
#include "can.h"
#include "uart.h"
#include "cmd_controller.h"
#include "pin_assign.h"



void sampel()
{
    while (1) 
    {
		vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}


void app_main()
{
	init_lin ();
	init_can ();
	init_uart();
	init_cmd();
	
	while(1)
	{
		// 設定完了し、実行指示を受信するまで待機
		while( CMD_ST_EXEC != exec_cmd() ){}

		//send_lin();
		//send_can();
		send_uart();
	}
}
