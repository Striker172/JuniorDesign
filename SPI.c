/*
 * File:   SPI.c
 * Author: travi
 *
 * Created on January 29, 2026, 9:21 AM
 */


#include "xc.h"
#include "SPI.h"
void setup_SPI(){
    _TRISB9 = 0; //Chip select
    _LATB9 = 1;
    SPI1CON1 = 0x3D;
    SPI1CON2 = 0x01;
    SPI1STAT = 0x800C;
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
        RPOR4bits.RP8R = 0x0007;    //RB8->SPI1:SDO1
        RPOR3bits.RP7R = 0x0008;    //RB7->SPI1:SCK1OUT
        RPINR20bits.SDI1R = 0x0006;    //RB6->SPI1:SDI1
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS
}
void send_sequence(unsigned char* value){
    _LATB9 = 0;
    while(*value != '\0'){
        SPI1BUF = *value++;
        while(!SPI1STATbits.SPIRBF);          // Send character
    }
    SPI1BUF = '\0';					// write to buffer for TX
    while(!SPI1STATbits.SPIRBF);	// wait for transfer to complete
    _LATB9 = 1;
}