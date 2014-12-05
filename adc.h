/* 
 * File:   adc.h
 * Author: jz95c2
 *
 * Created on March 25, 2014, 6:05 PM
 */

#ifndef ADC_H
#define	ADC_H

#ifdef	__cplusplus
extern "C" {
#endif




// Analog Open pin CH 0
#define OPENAN0 (0) // analog pin not in use

// Analog Open pin CH 1
#define OPENAN1 (1) // analog pin not in use

// Analog Open pin CH 2
#define OPENAN2 (2) // analog pin not in use 

// Button for measuring stroks per min
#define BTNIN   (3)

// TC1047 Temperature sensor with voltage output
#define TSENS   (4)

// 10k potentiometer connected to AN5 input
#define POT     (5)

// Analog inputs for Explorer16 POT, TSENS and BTNIN
#define AINPUTS (0xffc7)




void initADC( int amask);
int readADC( int ch);








#ifdef	__cplusplus
}
#endif

#endif	/* ADC_H */

