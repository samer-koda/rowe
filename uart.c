/*
 * File:   uart.c
 * Author: jz95c2
 *
 * Created on February 2, 2014, 4:16 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "p24FJ128GA010.h"



#ifdef _ISR
#undef _ISR
#define _ISR __attribute__((interrupt, auto_psv))
#endif

unsigned char UartRxData;

void _ISR _U2RXInterrupt(void)
{
    while ((U2STA & 0x0001) > 0)
    {
        UartRxData = U2RXREG;
    }
    //clear the interrupt flag
    _U2RXIF = 0;
}


void UartInit(void)
{
    U2MODE = 0x0208;//Enable HW flow control so that TeraTerm on PC side can receive.
    U2BRG = 34;//Baud=115200
    U2STA = 0x0400;
    _U2TXIE = 0;
    _U2RXIE = 1;
    _U2RXIP = 4;
    _U2TXIP = 0;
    _U2RXIF = 0;
    _U2TXIF = 0;
    U2MODE = 0x8208;//Enable HW flow control so that TeraTerm on PC side can receive.
    U2STA = 0x0400;
}

unsigned char GetUartRxData(unsigned char *buf, unsigned char buf_size)
{
    if (UartRxData>0)
    {
        *buf = UartRxData;
        UartRxData = 0;
        return 1;
        //UartRxData = 0;
    }
    else
    {
        return 0;
    }
}

void UartTxOneChar(unsigned char tx_data)
{
    while (U2STA & 0x0200)
    {
        //Wait for tx fifo free
    }
    U2TXREG = tx_data;
}


