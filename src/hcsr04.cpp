/*
 * The MIT License (MIT)

Copyright (c) 2015 Jetsonhacks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <jetsonGPIO.h>
#include "hcsr04.h"

// Set the GPIO pins for the trigger and the echo to the HC-SR04
HCSR04::HCSR04(jetsonGPIO triggerGPIO, jetsonGPIO echoGPIO) {
    hcsr04Trigger = triggerGPIO ;
    hcsr04Echo = echoGPIO ;
}

HCSR04::~HCSR04() {
}

// Make GPIO available in user space
void HCSR04::exportGPIO ( void ) {
    gpioExport(hcsr04Trigger) ;
    gpioExport(hcsr04Echo) ;
}

void HCSR04::unexportGPIO ( void ) {
    gpioUnexport(hcsr04Trigger) ;
    gpioUnexport(hcsr04Echo) ;
}

void HCSR04::setDirection ( void ) {
    // Set the pin directions - Trigger is output, Echo is input
    gpioSetDirection(hcsr04Trigger,outputPin) ;
    gpioSetDirection(hcsr04Echo,inputPin) ;
}

// Send the trigger sound from the HC-SR04
bool HCSR04::triggerPing ( void ) {
    unsigned int maxEcho = MAX_SENSOR_DISTANCE*ROUNDTRIP_CM + MAX_SENSOR_DELAY ;
    unsigned int echoValue = low ;
    // figure out duration of the pulse
    struct timeval tval_before, tval_after, tval_result;
    // Set trigger low to get ready to start
    gpioSetValue(hcsr04Trigger,low) ;
    usleep(4) ;
    // Pull trigger high for 10 microseconds to initiate echo location
    gpioSetValue(hcsr04Trigger,high) ;
    usleep(10) ;
    gpioSetValue(hcsr04Trigger,low) ;
    gettimeofday(&tval_before, NULL);

    gpioGetValue(hcsr04Echo,&echoValue) ;
    if (echoValue) {
        return false ;  // Previous echo not finished
    }
    while (echoValue != high) {
        gpioGetValue(hcsr04Echo,&echoValue) ;
        gettimeofday(&tval_after, NULL);
        timersub(&tval_after, &tval_before, &tval_result);
        if (tval_result.tv_usec > maxEcho) {
            return false ;
        }
    }
    return true ;
}

// Return the duration of an echo from the HC-SR04 in uSecs
// The duration can then be converted to distance
// Divide by 58 for Centimeters
// Divide by 148 for Inches
// A return of 0 is NO_ECHO ; Objects are either too close or too far away to detect
unsigned int HCSR04::ping ( void ) {
    // trigger a ping, then wait for the echo
    if (!triggerPing()) {
        // Timed out ; No objects or objects too close to sensor to measure
        return NO_ECHO ;
    } else {
        unsigned int maxEcho = MAX_SENSOR_DISTANCE*ROUNDTRIP_CM + MAX_SENSOR_DELAY ;
        unsigned int echoValue = high ;
        // figure out duration of the pulse
        struct timeval tval_before, tval_after, tval_result;
        gettimeofday(&tval_before, NULL);
        while (echoValue != low) {
            gpioGetValue(hcsr04Echo,&echoValue) ;
            if (echoValue==low) {
                break;
            }

            gettimeofday(&tval_after, NULL);
            timersub(&tval_after, &tval_before, &tval_result);
            if (tval_result.tv_usec > maxEcho) {
                return NO_ECHO ;
            }
        }
        gettimeofday(&tval_after, NULL);
        timersub(&tval_after, &tval_before, &tval_result);
        // convert to microseconds
        // second conversion not necessary
        return tval_result.tv_sec*1000000+tval_result.tv_usec ;
    }
}

unsigned int HCSR04::calculateMedian (int count, unsigned int sampleArray[]) {
    unsigned int tempValue ;
    int i,j ;   // loop counters
    for (i=0 ; i < count-1; i++) {
        for (j=0; j<count; j++) {
            if (sampleArray[j] < sampleArray[i]) {
                // swap elements
                tempValue = sampleArray[i] ;
                sampleArray[i] = sampleArray[j] ;
                sampleArray[j] = tempValue ;
            }
        }
    }
    if (count%2==0) {
        // if there are an even number of elements, return mean of the two elements in the middle
        return ((sampleArray[count/2] + sampleArray[count/2-1]) / 2.0) ;
    } else {
        // return the element in the middle
        return (sampleArray[count/2]) ;
    }
}

// Return the median duration of a number of pings
unsigned int HCSR04::pingMedian (int iterations) {
    unsigned int pings[iterations] ;
    unsigned int lastPing ;
    int index = 0 ;
    int samples = iterations ;
    int cursor = 0 ;
    pings[0] = NO_ECHO ;
    while (index < iterations) {
        lastPing = ping() ;
        if (lastPing != NO_ECHO) {
            // Add this to the samples array
            pings[cursor] = lastPing ;
            cursor ++ ;
        } else {
            // Nothing detected, don't add to samples.
            samples -- ;
        }
        index ++ ;
        usleep(1000) ; // delay a millisecond between pings
    }
    // Figure out the median of the samples
    if (samples == 0) return NO_ECHO ;
    return calculateMedian(samples,pings) ;
}
