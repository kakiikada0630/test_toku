#ifndef CAN_H
#define CAN_H

void init_can();
void send_can();
void set_can_baudrate(uint32_t baud);
uint32_t recv_can(uint8_t* p_rec, uint32_t size);

#endif
