/*
 * file: testpreempt.c
 */
#include <8051.h>
#include "preemptive.h"
/*
* @@@ [2pt]
* declare your global variables here, for the shared buffer
* between the producer and consumer.
* Hint: you may want to manually designate the location for the
* variable. you can use
* _data __at (0x30) type var;
* to declare a variable var of the type
*/
#define BUFFER_SIZE 3
__idata __at (0x2D) char buffer[BUFFER_SIZE];
__idata __at (0x3A) char buf1;
__idata __at (0x38) char buf2;
__idata __at (0x3B) char produce;
__idata __at (0x3C) char consume;
__idata __at (0x3D) char total;
__idata __at (0x35) char mutex;
__idata __at (0x36) char full;
__idata __at (0x37) char empty;

/* [8 pts] for this function
* the producer in this test program generates one characters at a
* time from 'A' to 'Z' and starts from 'A' again. The shared buffer
* must be empty in order for the Producer to write.
*/
void Producer1( void ) {
    /*
    * @@@ [2 pt]
    * initialize producer data structure, and then enter
    * an infinite loop (does not return)
    */
    buf1 = 'A';
    for (;;) {
    /* @@@ [6 pt]
    * wait for the buffer to be available,
    * and then write the new data into the buffer */
        SemaphoreWait(empty);
        SemaphoreWait(mutex);
        if (total < 3) {
            buffer[produce] = buf1;
            total ++;
            if (produce == 2) produce = 0;
            else produce ++;
            // produce = ( produce  == 2) ? 0 : produce + 1;
            // buf1 = (buf1 == 'Z') ? 'A' : buf1 + 1;
            if (buf1 == 'Z') buf1 = 'A';
            else buf1 ++;
        }
        SemaphoreSignal(mutex);
        SemaphoreSignal(full);
    }
}
void Producer2( void ) {
    /*
    * @@@ [2 pt]
    * initialize producer data structure, and then enter
    * an infinite loop (does not return)
    */
    buf2 = '0';
    for (;;) {
    /* @@@ [6 pt]
    * wait for the buffer to be available,
    * and then write the new data into the buffer */
        SemaphoreWait(empty);
        SemaphoreWait(mutex);
        if (total < 3) {
            buffer[produce] = buf2;
            total ++;
            if (produce == 2) produce = 0;
            else produce ++;
            // produce = (produce  == 2) ? 0 : produce + 1;
            // buf2 = (buf2 == '9') ? '0' : buf2 + 1;
            if (buf2 == '9') buf2 = '0';
            else buf2 ++ ;
        }
        SemaphoreSignal(mutex);
        SemaphoreSignal(full);
    }
}

/* [10 pts for this function]
* the consumer in this test program gets the next item from
* the queue and consume it and writes it to the serial port.
* The Consumer also does not return.
*/
void Consumer( void ) {
    /* @@@ [2 pt] initialize Tx for polling */
    TMOD |= 0x20;
    TH1 = (char)-6;
    SCON = 0x50;
    TR1 = 1;
    for (;;) {
    /* @@@ [2 pt] wait for new data from producer
    * @@@ [6 pt] write data to serial port Tx,
    * poll for Tx to finish writing (TI),
    * then clear the flag
    */
        SemaphoreWait(full);
        SemaphoreWait(mutex);
        if (total > 0) {
                SBUF = buffer[consume];
                total --;
                consume =  (consume == 2) ? 0 : consume + 1;
                while (!TI) { }
                TI = 0;
        }
        SemaphoreSignal(mutex);
        SemaphoreSignal(empty);
    }
}
/* [5 pts for this function]
* main() is started by the thread bootstrapper as thread-0.
* It can create more thread(s) as needed:
* one thread can acts as producer and another as consumer.
*/
void main( void ) {
    /*
    * @@@ [1 pt] initialize globals
    * @@@ [4 pt] set up Producer and Consumer.
    * Because both are infinite loops, there is no loop
    * in this function and no return.
    */
    buffer[0] = buffer[1] = buffer[2] = 0;
    consume = produce = total = 0;
    SemaphoreCreate(mutex, 1);
    SemaphoreCreate(full, 0);
    SemaphoreCreate(empty, 3);
    ThreadCreate(&Producer1);
    ThreadCreate(&Producer2);
    ThreadCreate(&Consumer);
    // ThreadExit();
}

void _sdcc_gsinit_startup( void ) {
    __asm
    ljmp _Bootstrap
    __endasm ;
}
void _mcs51_genRAMCLEAR( void ) {}
void _mcs51_genXINIT( void ) {}
void _mcs51_genXRAMCLEAR( void ) {}
void timer0_ISR(void) __interrupt(1) {
    __asm
    ljmp _myTimer0Handler
    __endasm;
}