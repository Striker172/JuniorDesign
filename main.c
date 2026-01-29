#include "xc.h"
#include "UART.h"
#include "circularBuff.h"
#include <stdio.h>
// CW1: FLASH CONFIGURATION WORD 1 (see PIC24 Family Reference Manual 24.1)
#pragma config ICS = PGx1          // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config FWDTEN = OFF        // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config GWRP = OFF          // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF           // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF        // JTAG Port Enable (JTAG port is disabled)

// CW2: FLASH CONFIGURATION WORD 2 (see PIC24 Family Reference Manual 24.1)
#pragma config I2C1SEL = PRI       // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF       // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON       // Primary Oscillator I/O Function (CLKO/RC15 functions as I/O pin)
#pragma config FCKSM = CSECME      // Clock Switching and Monitor (Clock switching is enabled, 
                                       // Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL      // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))

volatile char num[20] = {'0'};
void __attribute__((__interrupt__, __auto_psv__)) _ADC1Interrupt(void) {
    //It takes 2.75microseconds to convert the data with the current settings
    _AD1IF = 0;
    //Interrupt for when the ADC converison is done
    putVal(ADC1BUF0);
}
void __attribute__((interrupt, auto_psv)) _T2Interrupt(){
    _T2IF = 0; //Sends the value every 0.1 seconds
    sprintf(num,"%6.4f",(3.3/1024)*getAvg());
    sendValue(num);
}
void setADC(){
     AD1PCFG = 0x9fff;
     CLKDIVbits.RCDIV = 0;
    _TRISA0 = 1;              //This is defined as the input pin
    
    AD1PCFGbits.PCFG0 = 0;    //This sets the pin to Analog
    AD1CON2bits.VCFG = 0b100; //Internal pin sampling
    AD1CON3bits.ADCS = 1;     //Converison clock period 2*PR3
    AD1CON1bits.SSRC = 0b010; //TMR3 compare matching
    AD1CON3bits.SAMC = 10;    //Auto Sample Time Bits
    AD1CON1bits.FORM = 0b00;  //Integer output

    AD1CON1bits.ASAM = 0b1;   //The sampling is enabled after conversion
    AD1CON2bits.SMPI = 0b0;   //Triggers Interrupts after every conversion
    AD1CON1bits.ADON = 1;     //Enabled module
    
    TMR3=0;
    T3CON = 0;
//    T3CONbits.TCKPS = 0b10; //16Mhz 
    T3CONbits.TCKPS = 0b01; //128Mhz
    PR3 = 15624; //IDK 
    TMR2 = 0;
    T2CON = 0;
    T2CONbits.TCKPS = 0b10;
    PR2 = 24999;
    _T2IF = 0;
    _T2IE = 1;
    _AD1IF = 0;
    _AD1IE = 1;
    setupUART();
}
int main(void) {
    setADC();
    T3CONbits.TON = 1;
    T2CONbits.TON = 1;
//    unsigned char* message = ".425";
    while(1){
//        sendValue(message);
    }
    return 0;
}
