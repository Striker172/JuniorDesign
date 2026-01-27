/* 
 * File:   UART.h
 * Author: travi
 *
 * Created on January 27, 2026, 9:09 AM
 */

#ifndef UART_H
#define	UART_H

#ifdef	__cplusplus
extern "C" {
#endif

    void setupUART();
    void sendData(uint8_t message);

#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */

