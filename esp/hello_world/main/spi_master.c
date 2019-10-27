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
#include "esp_system.h"
#include "driver/spi_master.h"
#include "spi_master.h"
#include "freertos/task.h"
#include "pin_assign.h"


#define CMD_SIZE 2

static spi_device_handle_t    spi = NULL;
spi_transaction_t             trans;
spi_bus_config_t              buscfg;
spi_device_interface_config_t devcfg;
uint8_t*                      cmd;


void send_spi_data(int32_t data)
{
	for( int j=0 ; j<CMD_SIZE ; j++ )
	{
		cmd[j] = data;
	}
	
    esp_err_t ret;
    memset(&trans, 0, sizeof(trans));  //Zero out the transaction

    trans.length=8*CMD_SIZE;           //Command is 8 bits
    trans.tx_buffer=cmd;              //The data is the cmd itself
    trans.user=(void*)0;               //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi, &trans);  //Transmit!
    assert(ret==ESP_OK);               //Should have had no issues.
}

static void spi_sender(void *pvParameters)
{
	static uint64_t base = 0;
	static uint64_t cur  = 0;
	static int      i    = 0;

	while(1)
	{
		base=esp_timer_get_time();

		for(i=0xff; i>0xe1 ; i--)
		{
			send_spi_data(i);
		}
		
		while(1)
		{
			cur = esp_timer_get_time();

			if(base+5000 < cur)
			{
				break;
			}

			//vTaskDelay(1);
			ets_delay_us(50);
		}
	}
}


void init_spi_master()
{
    buscfg.miso_io_num    = SPI_MASTER_MISO_PIN;
    buscfg.mosi_io_num    = SPI_MASTER_MOSI_PIN;
    buscfg.sclk_io_num    = SPI_MASTER_SCLK_PIN;
    buscfg.quadwp_io_num  = -1;
    buscfg.quadhd_io_num  = -1;
    //buscfg.flags          = SPICOMMON_BUSFLAG_MASTER;
    buscfg.max_transfer_sz= 100*8; //in bits (this is for 32 bytes. adjust as needed)

    devcfg.clock_speed_hz= 1*1000*1500;//this is for 1MHz. adjust as needed
    devcfg.mode          = 0;
    devcfg.spics_io_num  = SPI_MASTER_CS_PIN;
    devcfg.queue_size    = 1;//how many transactions will be queued at once
    devcfg.flags         = SPI_DEVICE_NO_DUMMY;

    spi_bus_initialize(VSPI_HOST, &buscfg, 2   );
    spi_bus_add_device(VSPI_HOST, &devcfg, &spi);
    
    cmd = (uint8_t*)heap_caps_malloc(CMD_SIZE, MALLOC_CAP_DMA);

    //Create a task to handler UART event from ISR
    //xTaskCreate(spi_sender, "spi_master", 1024, NULL, 12, NULL);
    xTaskCreatePinnedToCore(spi_sender, "spi_master", 1024, NULL, 12, NULL, 1);

}

