/* I2C Select Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
//#include "sdkconfig.h"
#include "i2c_server.h"
#include "pin_assign.h"

#define ADC_MCP4728_ADDRESS 0xC0

#define ACK_CHECK_EN              0x1               /*!< I2C master will check ack from slave*/
#define I2C_MASTER_SCL_IO         I2C_MASTER_SCK    /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO         I2C_MASTER_SDA    /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM            I2C_NUM_0         /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ        50*1000           /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                 /*!< I2C master doesn't need buffer */


/**
 * @brief i2c master initialization
 */
esp_err_t init_i2c()
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}

/**
 * @brief test code to operate on BH1750 sensor
 *
 * 1. set operation mode(e.g One time L-resolution mode)
 * _________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write 1 byte + ack  | stop |
 * --------|---------------------------|---------------------|------|
 * 2. wait more than 24 ms
 */
esp_err_t send_i2c_mcp4728(mcp_4728_ch adc_ch, uint32_t data)
{
    int ret;
    int i;

	uint8_t send_byte[4] = {0,0,0,0};
	
	send_byte[0]  = ADC_MCP4728_ADDRESS | I2C_MASTER_WRITE;
	send_byte[1]  = 0x40 | ( (uint8_t)adc_ch << 1 );
	send_byte[2]  = (uint8_t)(data >> 8);
	send_byte[3]  = (uint8_t)(data & 0x000000ff);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
	for(i = 0 ; i<4 ; i++)
	{
	    i2c_master_write_byte(cmd, send_byte[i], ACK_CHECK_EN);
	}
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}


