/* 
 * File:   SPI.h
 * Author: travi
 *
 * Created on January 29, 2026, 9:21 AM
 */

#ifndef SPI_H
#define	SPI_H

#ifdef	__cplusplus
extern "C" {
#endif

void setup_SPI();
void send_sequence(unsigned char* value);
void sendBytes(uint8_t data);
#ifdef	__cplusplus
}
#endif

#endif	/* SPI_H */

