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
#define HEARTBEATSMPL 4
#define SECPR2 65535
#define BEEPDURATION 6249
struct heartTimes{
    volatile long unsigned int timerValue;
    volatile int overflowT1;
};
enum TMR1FLAGS { SECFLAG = 1};
enum FLAGS {SENDDATA =1, PUTVALBUF, GETVALBUF, HEARTCHK };
volatile long unsigned int intervalHeart = 0;
volatile char num[20] = {'0'};
volatile int heart_index = 0;
volatile struct heartTimes heartbeats[HEARTBEATSMPL] = {0};
volatile int value = 0;
volatile int prevValue = 0;
volatile int overflowT1 = 0;
volatile int beepInterval = 0;
//Index 0: Send Data, Index 1: Checks for Buzzer, Index 2: TMR1_setting
volatile int Flags[3] = {0};  
void __attribute__((__interrupt__, __auto_psv__)) _ADC1Interrupt(void) {
    //It takes 2.75microseconds to convert the data with the current settings
    _AD1IF = 0;
    //Interrupt for when the ADC converison is done
    value = ADC1BUF0;
    Flags[0] = SENDDATA;
    if(value >= 600 && prevValue < 600){ //This should be changed
        Flags[1] = PUTVALBUF;
        prevValue = value;
    } else if(value < 600 && prevValue >= 600 && Flags[1] != HEARTCHK){
        Flags[1] = GETVALBUF;
        prevValue = 0;
    }
    prevValue = value;
}
//Timer duration is 1second basically
void __attribute__((interrupt, auto_psv)) _T1Interrupt(){
    _T1IF = 0;
    overflowT1++;
}
void __attribute__((interrupt, auto_psv)) _T2Interrupt(){
    _T2IF = 0;
    if(beepInterval >0){
        beepInterval--;
        if(beepInterval == 0){
            OC1RS = 0;
        }
    }
}
void setADC(){
     AD1PCFG = 0x9fff;
     CLKDIVbits.RCDIV = 0;
    _TRISA0 = 1;              //This is defined as the input pin for the ADC
    _TRISB7 = 0;              //OUTPUT
    //ADC
    AD1PCFGbits.PCFG0 = 0;    //This sets the pin to Analog
    AD1CON2bits.VCFG = 0b100; //Internal pin sampling
    AD1CON3bits.ADCS = 1;     //Converison clock period 2*PR3
    AD1CON1bits.SSRC = 0b010; //TMR3 compare matching
    AD1CON3bits.SAMC = 10;    //Auto Sample Time Bits
    AD1CON1bits.FORM = 0b00;  //Integer output
    AD1CON1bits.ASAM = 0b1;   //The sampling is enabled after conversion
    AD1CON2bits.SMPI = 0b0;   //Triggers Interrupts after every conversion
    AD1CON1bits.ADON = 1;     //Enabled module
    
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
    RPOR3bits.RP7R = 0x0012;                //SETs Output compare to RP7
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS
    //Timer 3
    TMR3=0;
    T3CON = 0;
//    T3CONbits.TCKPS = 0b10; //16hz 
    T3CONbits.TCKPS = 0b01; //128hz
    PR3 = 15624; //IDK 
    //Timer 1
    TMR1 = 0;
    T1CON = 0;
    T1CONbits.TCKPS = 0b11; 
    PR1 = SECPR2;
    Flags[2] = SECFLAG;
    _T1IF = 0;
    _T1IE = 1;
    _AD1IF = 0;
    _AD1IE = 1;
    //Output Compare 
    TMR2 = 0;
    T2CON = 0x0010;
    PR2 = 39999;
    OC1CON = 0;
    OC1R = 2500;
    OC1RS = 0;
    OC1CONbits.OCTSEL = 0;
    OC1CONbits.OCM = 0b110;
    _T2IF = 0;
    _T2IE = 1;
    setupUART();
}
int main(void) {
    setADC();
    T1CONbits.TON = 1;
    T2CONbits.TON = 1;
    T3CONbits.TON = 1;
    while(1){
        if(Flags[0] == SENDDATA){
            _AD1IE = 0;   // Disable ADC interrupt
            sendValue(value);
             Flags[0] = 0;
            _AD1IE = 1;   // Re-enable
        }
        if(Flags[1] == PUTVALBUF){
            T1CONbits.TON = 1;
            putVal((TMR1) +(SECPR2*overflowT1));
            Flags[1] = 0;
            TMR1 = 0;
            overflowT1 = 0;
        } else if (Flags[1] == GETVALBUF){
            heartbeats[heart_index].timerValue = getAvg();
            heartbeats[heart_index].overflowT1 = overflowT1;
            Flags[1] = (heart_index == 3)? HEARTCHK : 0;
            heart_index = (1+heart_index)%HEARTBEATSMPL;
        }
        if(Flags[1] == HEARTCHK){
            //Idk if i should add an interval check for the heart beats
            
            for(int i = 0; i < HEARTBEATSMPL;i++){
                intervalHeart += (heartbeats[i].timerValue +(SECPR2 *heartbeats[i].overflowT1));
                heartbeats[i].overflowT1 = 0;
            }
//            overflowT1 = 0;
            intervalHeart /= HEARTBEATSMPL;
            OC1RS = 3600;
            beepInterval = 3;
            Flags[1] = 0;
        }
    }
    return 0;
}
