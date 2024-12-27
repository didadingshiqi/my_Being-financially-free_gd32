#ifndef USART_H
#define USART_H

#include "gd32f10x.h"

#define USART0_RDATA_ADDRESS      ((uint32_t)&USART_DATA(USART0))

extern uint8_t rxbuffer[256];
extern uint8_t rx_count;
extern uint8_t tx_count;
extern uint8_t receive_flag;


void dma0_config(void);
void usart0_config(void);
void usart0_send_data(uint8_t* buf, uint8_t len);
void usart0_nvic_config(void);

#endif /* SYS_TICK_H */
