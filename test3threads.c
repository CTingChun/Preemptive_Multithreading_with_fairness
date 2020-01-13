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
#define BUFFER_SIZE 6
__idata __at (0x2D) char buffer[BUFFER_SIZE];
__idata __at (0x3A) char buf;
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
void Producer( void ) {
    /*
    * @@@ [2 pt]
    * initialize producer data structure, and then enter
    * an infinite loop (does not return)
    */
    buf = 'A';
    for (;;) {
    /* @@@ [6 pt]
    * wait for the buffer to be available,
    * and then write the new data into the buffer */
        SemaphoreWait(empty);
        SemaphoreWait(mutex);
        if (total < 3) {
                buffer[produce] = buf;
                total ++;
                if (produce == 2) 
                        produce = 0;
                else 
                        produce ++;
                if (buf == 'Z') 
                        buf = 'A';
                else 
                        buf ++;
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
                if (consume == 2) 
                        consume = 0;
                else 
                        consume ++;
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
    ThreadCreate(Producer);
    ThreadCreate(Consumer);
    ThreadExit();
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