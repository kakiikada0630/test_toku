/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "driver/spi_slave.h"
#include "freertos/task.h"
#include "spi_slave_peri.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "pin_assign.h"

#define  QUE_SIZE   40
#define  BUF_SIZE    2 * QUE_SIZE

static spi_slave_transaction_t      trans[QUE_SIZE];
//static spi_slave_transaction_t      *result;
spi_bus_config_t                    bus_config;
spi_slave_interface_config_t        intterface_config;
uint16_t*                           spi_slave_rx_buf;
uint16_t*                           receive_buf;

uint32_t get_data;
uint32_t get_data1;

static uint32_t start_index=0;     // 最初のデータ格納インデックス
static uint32_t last_index =0;      // 最新の受信データを格納したインデックス
static uint64_t last_time;       // 最新のデータを受信したチック


static void spi_slave_tans_done(uint32_t* data)
{
	static uint64_t cur_time;
	//static uint32_t buff;

	cur_time    = esp_timer_get_time();
	last_index  =(last_index+1)%QUE_SIZE;

	receive_buf[last_index]= *((uint16_t *)data);

	// 最後の受信時間から500us以上空いて信号を受信した場合、新しい
	// 命令列の開始と判断する。
	if(500 < (cur_time - last_time) )
	{
		start_index = last_index;
	}

	last_time  = cur_time;
}

void init_spi_slave()
{
	bus_config.mosi_io_num   = SPI_SLAVE_MOSI_PIN;
	bus_config.miso_io_num   = SPI_SLAVE_MISO_PIN;
	bus_config.sclk_io_num   = SPI_SLAVE_SCLK_PIN;
	bus_config.quadwp_io_num = -1;
	bus_config.quadhd_io_num = -1;
	bus_config.flags         = SPICOMMON_BUSFLAG_SLAVE;
	bus_config.max_transfer_sz = 8*BUF_SIZE;

	intterface_config.spics_io_num = SPI_SLAVE_CS_PIN;
	intterface_config.flags        = 0;
    intterface_config.queue_size   = QUE_SIZE;
	intterface_config.mode         = 0;
	intterface_config.post_trans_cb= spi_slave_tans_done;

	receive_buf      = (uint16_t*)heap_caps_malloc(BUF_SIZE, MALLOC_CAP_DMA);
	spi_slave_rx_buf = (uint16_t*)heap_caps_malloc(BUF_SIZE, MALLOC_CAP_DMA);
	spi_slave_initialize( HSPI_HOST, &bus_config, &intterface_config, 0 );

	for(int i=0 ; i<1 ; i++)
	{
		trans[i].rx_buffer = spi_slave_rx_buf+i;
		trans[i].user      = (void *)i;
		spi_slave_queue_trans( HSPI_HOST, &trans[i], portMAX_DELAY);
	}

	//-----------------------------
	//デバッグ用ピン
	gpio_config_t config = {
		.pin_bit_mask = (((uint64_t) 1) << DEBUG_OUT1_PIN),
		.mode         = GPIO_MODE_OUTPUT,
		.pull_up_en   = 0,
		.pull_down_en = 0,
		.intr_type    = GPIO_INTR_DISABLE
	};
	gpio_config( &config );
	//-----------------------------
}

int64_t get_spi_ratest_time()
{
	return last_time;
}


uint32_t get_spi_data(uint16_t* p_data, uint32_t buf_size)
{
    uint64_t c_time;
    uint64_t l_time;
    static uint32_t start;
    static uint32_t last=0;
    
    // 一連の命令列の受信完了を待つ
    //   500us SPIデータ受信が途切れた場合に一連の命令列受信完了と判断
    while(1)
    {
		c_time = esp_timer_get_time();
		l_time = last_time;

		if(500+l_time < (c_time))
		{
			//直前の最終Indexと同じなら、新規データ未受信と判断し、
			//開始インデックスを終端と同じとする。
			start = ( last == last_index)? last : start_index;
			last  = last_index;

			break;
		}
		//ets_delay_us(200);
		vTaskDelay(1);

	}

	uint32_t last_index_tmp = ( last < start )? last+QUE_SIZE : last;
	uint32_t size           = last_index_tmp - start +1;

    // 受信データ数が与えるバッファより大きい場合
    // バッファサイズを超えないよう調整する。
	if( buf_size < size ){ size = buf_size; }

    // 通常の利用ケースにおいて、受信サイズが1以下となることはないため、
    // 1以下の場合は受信データなしとみなす。
	if( size < 2 ){ size = 0; }

	for(uint32_t i = 0 ; i<size ; i++ )
	{
		p_data[i] = receive_buf[(start+i)%QUE_SIZE];
	}

	return size;
}