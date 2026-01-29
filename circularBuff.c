/*
 * File:   circularBuff.c
 * Author: travi
 *
 * Created on April 9, 2025, 11:21 AM
 */


#include "xc.h"
#include "circularBuff.h"
#define sampleSize 128
volatile unsigned long buff[BUFFER_SIZE] = {0};
volatile int bufferIndex = sampleSize;
volatile int bufferTail = 0;
/*
 * Puts the value into the circular buffer
 * @param newValue(int) : the value to be put into the buffer
 */
void putVal(int newValue){
    buff[bufferIndex] = newValue;
    bufferTail = (bufferTail+1)%BUFFER_SIZE;
    bufferIndex = (bufferIndex+1)%BUFFER_SIZE;
}
/*
 * Gets the average from the circular buffer by take the index and adding 128 to it each time it's called
 * @returns sum (unsigned long) : The average of the value stored within the buffer.
 */
unsigned long int getAvg(){
    unsigned long sum = 0;
    for(int i =0; i < sampleSize;i++){
        sum += buff[(bufferTail+i)%BUFFER_SIZE];
    }
    return (sum/sampleSize);
}
//void initBuffer(){
//    for(int i = 0; i<BUFFER_SIZE;i++){
//        buff[i] = 0;
//    }
//}