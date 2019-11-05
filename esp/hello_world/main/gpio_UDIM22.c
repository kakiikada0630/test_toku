/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "gpio_UDIM22.h"
#include "pin_assign.h"


static int64_t  latest_time     = 0;
static int64_t  start_time      = 0;
static int64_t  end_zero_time   = 0;
static int64_t  end_one_time    = 0;
static int32_t  per             = 0;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	int lev= gpio_get_level(GPIO_UDIM22);

	if( 0==lev )
	{
		latest_time   = esp_timer_get_time();
		end_one_time  = latest_time;
		int32_t bunsi = ( int32_t )(end_one_time - end_zero_time);
		int32_t bunbo = ( int32_t )(end_one_time - start_time   );
		per = bunsi*10000 / bunbo;
		start_time   = end_one_time;
	}
	else
	{
		latest_time   = latest_time;
		end_zero_time = esp_timer_get_time();
	}
}

void init_gpio_UDIM22()
{
	gpio_config_t config = {
		.pin_bit_mask = (((uint64_t) 1) << GPIO_UDIM22),
		.mode         = GPIO_MODE_INPUT,
		.pull_up_en   = 0,
		.pull_down_en = 0,
		.intr_type    = GPIO_INTR_ANYEDGE
	};
    gpio_config( &config );

    gpio_isr_handler_add(GPIO_UDIM22, gpio_isr_handler, (void*) GPIO_UDIM22);
}

int32_t get_percent_UDIM22()
{
	int64_t cur_time;
	int32_t buf=0;

	cur_time = esp_timer_get_time();
	buf = (int32_t)(cur_time - latest_time);

	//最新の割り込み時間から7ms以上経過している場合、Dutyが
	//0%か100.00%に張り付いていると判断する。
	if( buf > 7000 )
	{
		per = ( gpio_get_level(GPIO_UDIM22)==0 )? 0 : 10000;
	}

	return per;

}