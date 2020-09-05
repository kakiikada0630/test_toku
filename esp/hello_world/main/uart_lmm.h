#ifndef UART_LMM_H
#define  UART_LMM_H

void init_uart();
void send_uart();
void set_uart_baudrate(uint32_t baud);

uint32_t recv_uart(uint8_t* p_rec, uint32_t size);

#endif
