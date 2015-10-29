/*
The MIT License (MIT)

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
#include <jetsonGPIO.h>

#ifndef HCSR04_H_
#define HCSR04_H_


#define MAX_SENSOR_DISTANCE 500     // For the HC-SR04 this is probably around 400, add a little bit extra
#define ROUNDTRIP_CM        58      // uSeconds for sound pusle to travel 2 cm (roundtrip of 1 cm)
#define MAX_SENSOR_DELAY    5800
#define NO_ECHO             0

class HCSR04
{
public:
    jetsonGPIO hcsr04Trigger ;
    jetsonGPIO hcsr04Echo ;
    HCSR04(jetsonGPIO triggerGPIO=gpio165, jetsonGPIO echoGPIO=gpio166) ;
    ~HCSR04() ;
    // GPIO functions
    void exportGPIO ( void ) ;
    void unexportGPIO ( void ) ;
    void setDirection ( void ) ;
    // Pingin'
    bool triggerPing ( void ) ;			// Send out the trigger pulse to start an echo
                                        // Not usually used by application code
    unsigned int ping ( void ) ;		// return the duration of an echo pulse
    unsigned int pingMedian (int iterations) ;      // return the median duration of iterations # of pings
    // caluclation helpers
    unsigned int calculateMedian (int count, unsigned int sampleArray[]) ;
};



#endif // HCSR04_H_

