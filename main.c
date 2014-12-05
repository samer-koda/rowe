/* 
 * File:   main.c
 * Author: KODA
 *
 * Created on December 2, 2014, 2:21 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "p24FJ128GA010.h"
#include "lcd.h"
#include "eeprom.h"
#include "adc.h"
#include "uart.h"

//disable JTAG and Watchdog

_CONFIG1(JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF & FWDTEN_OFF & ICS_PGx2)
//set CPU clock to FRCPLL (32MHz)
_CONFIG2(POSCMOD_NONE & FNOSC_FRCPLL & OSCIOFNC_ON & FCKSM_CSECME)


#ifdef _ISR
#undef _ISR
#define _ISR __attribute__((interrupt, auto_psv))
#endif

#define HIGH 0xff
#define LOW  0x00

typedef enum RowingMechineState_t {
    ST_PowerUp = 0,
    ST_Rowing,
    ST_History,
} RowingMechineState_t;

typedef struct History_EEPROM_t {
    unsigned short Dist;
    unsigned short Cal;
    unsigned short Time;
} History_EEPROM_t;

//============================================================================
//static gloable variables
//============================================================================
static unsigned char tmr1_int_flag = 0;
static unsigned char cn15_int_flag = 0;
static unsigned char cn16_int_flag = 0;
static unsigned char cn19_int_flag = 0;

static RowingMechineState_t RoweState = ST_PowerUp;
static char LCDFirstLn[17];
static char LCDSecondLn[17];


//============================================================================
//Timer1 interrupt ISR
//============================================================================

void _ISR _T1Interrupt(void) {
    tmr1_int_flag = 1;
    //clear the interrupt flag
    _T1IF = 0;
}

//============================================================================
//Delay function using TIMER2, 16MHz, prescaler=256, one tick = 16us
//============================================================================

void mydelay_us(unsigned long period) {
    TMR2 = 0;
    while (TMR2 < (period >> 4));
}

//============================================================================
//Delay function using TIMER2, 16MHz, prescaler=256, one tick = 16us
//============================================================================

void mydelay_ms(unsigned long period) {
    TMR2 = 0;
    while (TMR2 < (period << 6));
}

//============================================================================
//TIMR1 init function
//============================================================================

void TMR1_init(void) {
    //Init Timer1, used to generate heart beat.
    TMR1 = 0; // clear the timer
    PR1 = 62500; // 1sec interrupt
    _T1IP = 4; // this is the default value anyway
    _T1IF = 0; // Clear int flag
    _T1IE = 1; // Enable int
    T1CON = 0x8030; // TMR1 runs, prescaler 1:256 Tcy=Tclk/2=16MHz
}

//============================================================================
//TIMR2 init function
//============================================================================

void TMR2_init(void) {
    //Init Timer2, used as free run timer
    T2CON = 0x8030; // TMR2 runs, prescaler 1:256 Tcy=Tclk/2=16MHz, one tick=16us
}

//============================================================================
//CN interrupt ISR
//============================================================================

void _ISR _CNInterrupt(void) {
    //clear the interrupt flag
    if ((PORTD & 0x0040) == 0) {
        cn15_int_flag = 1;
    } else if ((PORTD & 0x0080) == 0) {
        cn16_int_flag = 1;
    } else if ((PORTD & 0x2000) == 0) {
        cn19_int_flag = 1;
    }
    _CNIF = 0;
}

//============================================================================
//CN init function
//============================================================================

void CN_init(void) {
    _CNIP = 4; // this is the default value anyway
    _CNIF = 0; // Clear int flag
    _CNIE = 1; // Enable int
    _CN15IE = 1; //Enable RD6 (S3) interrupt
    _CN16IE = 1; //Enable RD7 (S6) interrupt
    _CN19IE = 1; //Enable RD13 (S4) interrupt
}

//============================================================================
//Proccess Btn s3 interrupt - Main Screen
//============================================================================

void ProcessS3Int(void) {
    switch (RoweState) {
        case ST_PowerUp:
            RoweState = ST_Rowing;
            break;
        case ST_Rowing:
            break;
        default:
            break;
    }
}

//============================================================================
//Proccess Btn s4 interrupt - History Screen
//============================================================================

void ProcessS4Int(void) {
    switch (RoweState) {
        case ST_PowerUp:
            RoweState = ST_History;
            break;
        case ST_Rowing:
            break;
        case ST_History:
            RoweState = ST_PowerUp;
            break;
        default:
            break;
    }
}

//============================================================================
//Proccess Btn s6 interrupt - Ready/start Screen
//============================================================================

void ProcessS6Int(void) {
    switch (RoweState) {
        case ST_PowerUp:
            break;
        case ST_Rowing:
            RoweState=ST_PowerUp;

            break;
        default:
            break;
    }
}

static unsigned short a2d;
float volt;
int count = 0;

void help(void) {
    time_t mytime;
    mytime = time(NULL);
    a2d = (unsigned short) readADC(BTNIN);
    volt = (float) a2d * 3.3 / 1023.0;
    memset(LCDFirstLn, 0, 17);
    memset(LCDSecondLn, 0, 17);
    LCDcmd(0x80); //Put `cursor to the beginning of 1st line.
    sprintf(LCDFirstLn, ctime(&mytime));
    putsLCD(LCDFirstLn);
    LCDcmd(0xc0); //Put cursor to the beginning of 2st line.
    sprintf(LCDSecondLn, "%2.1f        %d", volt, count);
    putsLCD(LCDSecondLn);
    if (volt > 2.0) {
        count++;
        mydelay_ms(200);
    }
}

int main(int argc, char** argv) {

    //----------------------------------------------------
    //Make sure clock runs at 32MHz
    CLKDIV = 0;

    //----------------------------------------------------
    //do not allow nested interrupts
    _NSTDIS = 1;

    //----------------------------------------------------
    //Init LED controls
    TRISA = 0xff00;
    PORTA = LOW;

    //----------------------------------------------------
    //Init PORTD for detect button press
    TRISD = (TRISD | 0x20c0);

    //----------------------------------------------------
    //Init TIMR1/2 for heart rate signal and free run timer
    TMR1_init();
    TMR2_init();

    //----------------------------------------------------
    //Init CN
    CN_init();

    //-----------------------------------------------------
    //Init LCD
    LCDinit();

    //-----------------------------------------------------
    // Init UART
    UartInit();

    //-----------------------------------------------------
    // Init ADC
    initADC(AINPUTS);





    while (1) {

        help();




        if (cn15_int_flag > 0) {//S3
            mydelay_ms(200);
            _RA0 = _RA0 ^ (HIGH);

            cn15_int_flag = 0;
        }

        if (cn16_int_flag > 0) {//S6
            mydelay_ms(200);
            _RA1 = _RA1 ^ (HIGH);
            LCDclr();
            cn16_int_flag = 0;
        }

        if (cn19_int_flag > 0) {//S4
            mydelay_ms(200);
            _RA2 = _RA2 ^ (HIGH);
            cn19_int_flag = 0;
        }
    }
    return (EXIT_SUCCESS);
}

