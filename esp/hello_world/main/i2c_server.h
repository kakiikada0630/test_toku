#ifndef I2C_SERVER_H
#define  I2C_SERVER_H

typedef enum MCP4728CH
{
	MCP4728CH_0,
	MCP4728CH_1,
	MCP4728CH_2,
	MCP4728CH_3,
	MCP4728CH_MAX
} mcp_4728_ch;

esp_err_t init_i2c();
esp_err_t send_i2c_mcp4728(mcp_4728_ch adc_ch, uint32_t data);

#endif
