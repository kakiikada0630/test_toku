#ifndef LIN_H
#define LIN_H

void init_lin();
void send_lin();
void set_lin_baudrate(uint32_t baud);
uint32_t recv_lin(uint8_t* p_rec, uint32_t size);

#endif
