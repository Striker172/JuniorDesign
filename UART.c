/*
 * File:   UART.c
 * Author: travi
 *
 * Created on January 27, 2026, 9:09 AM
 */


#include "xc.h"
volatile unsigned char buffer[64];
volatile unsigned char front = 0;
volatile unsigned char back = 0;

void __attribute__((__interrupt__,__auto_psv__)) _U1RXInterrupt(void) {
    IFS0bits.U1RXIF = 0;
    buffer[front++] = U1RXREG;
    front &= 63;
}
void sendData(uint8_t message){
    while(!IFS0bits.U1TXIF);
    IFS0bits.U1TXIF = 0;
    U1TXREG = message;
}
void setupUART(){
    _TRISB6 = 0;                            //U1TX 
    _TRISB10 = 1;                           //U1RX
    U1MODE = 0;
    U1MODEbits.BRGH = 0;                    //Standard Mode
    U1BRG = 103;                             //9615 baud rate
    U1MODEbits.UARTEN = 1;                  //UART Enabled
    U1MODEbits.UEN = 0;                     //U1TX and U1RX are controlled via port hatches
    U1STAbits.UTXEN = 1;                    //Transmittion is enabled
    U1STAbits.UTXISEL0 = 
    U1MODEbits.PDSEL = 0b01;                //8-bit even parity
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
    _RP6R = 0x0003;                         //RB6->UART1:U1TX; See Table 10-3 on P109 of the datasheet
    _U1RXR = 10;                            //RB10->UART1:U1RX;
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS
    IFS0bits.U1RXIF = 0;                    //Enables the interrupt for receiving
    IEC0bits.U1RXIE = 1;
}
    