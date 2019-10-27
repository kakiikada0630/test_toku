#ifndef SPI_SLAVE_PERI_H
#define SPI_SLAVE_PERI_H

void     init_spi_slave();
int64_t  get_spi_ratest_time();
uint32_t get_spi_data(uint16_t* data, uint32_t buf_size);

#endif
