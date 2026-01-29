/* 
 * File:   circularBuff.h
 * Author: travi
 *
 * Created on April 9, 2025, 11:22 AM
 */

#ifndef CIRCULARBUFF_H
#define	CIRCULARBUFF_H
#define BUFFER_SIZE 1024
#ifdef	__cplusplus
extern "C" {
#endif
    void putVal(int newValue);
    unsigned long int getAvg();
    void initBuffer();
    extern volatile int bufferIndex;
    extern volatile unsigned long buff[BUFFER_SIZE];


#ifdef	__cplusplus
}
#endif

#endif	/* CIRCULARBUFF_H */

