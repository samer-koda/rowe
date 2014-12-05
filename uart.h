/* 
 * File:   uart.h
 * Author: JZ95C2
 *
 * Created on October 21, 2014, 6:48 PM
 */

#ifndef UART_H
#define	UART_H

#ifdef	__cplusplus
extern "C" {
#endif


void UartInit(void);
unsigned char GetUartRxData(unsigned char *buf, unsigned char buf_size);
void UartTxOneChar(unsigned char tx_data);



#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */

