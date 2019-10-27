#ifndef UART_LMM_H
#define  UART_LMM_H

void init_uart();
void send_uart();
uint32_t recv_uart(uint8_t* p_rec, uint32_t size);

#endif
