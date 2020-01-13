#include <8051.h>
#include "preemptive.h"

__idata __at (0x20) char savedSP[MAXTHREADS];
__idata __at (0x24) char threadBitmap[MAXTHREADS];
__idata __at (0x28) char ID;
__idata __at (0x29) char tmp;
__idata __at (0x2A) char i;
__idata __at (0x2B) char currentID;
__idata __at (0x2C) char managerID;

#define SAVESTATE \
{ __asm \
push ACC \
push B \
push DPL \
push DPH \
push PSW \
clr RS1 \
clr RS0 \
__endasm; \
savedSP[ID] = SP;\
}

#define RESTORESTATE \
{ SP = savedSP[ID];\
__asm \
pop PSW \
pop DPH \
pop DPL \
pop B \
pop ACC \
__endasm; \
}

extern void main( void );

void threadManager (void) {
    for (;;) {
        ID = ( ID == MAXTHREADS - 1 ) ? 0 : ID+1;
        if (threadBitmap[ID] > 0) break;
    }
    RESTORESTATE;
    __asm
        reti
    __endasm;
}

void myTimer0Handler (void) {
    SAVESTATE;
    tmp = ID;
    ID = managerID;
    RESTORESTATE;
    ID = tmp;
}

void Bootstrap( void ) {
    threadBitmap[0] = threadBitmap[1] = threadBitmap[2] = threadBitmap[3] = 0;
    TMOD = 0;
    IE = 0x82;
    TR0 = 1;
    managerID = ThreadCreate(&threadManager);
    ID = ThreadCreate(&main);
    RESTORESTATE;
}

ThreadID ThreadCreate(FunctionPtr fp) {

    EA = 0;
    for (i=0; i<MAXTHREADS; i++) if (!threadBitmap[i]) break;
    if (i == MAXTHREADS) return -1;
    threadBitmap[i] = 1;
    tmp = SP;
    SP = 0x3F + i*0x10;
    __asm
        mov a,DPL
        mov b,DPH
        mov dptr, #_ThreadExit
        push DPL
        push DPH
        push a
        push b
        mov a,#0x00
        push a
        push a
        push a
        push a
    __endasm;
    currentID = i<<3;
    __asm
        push _currentID
    __endasm;
    savedSP[i] = SP;
    SP = tmp;
    EA = 1;
    return i;
}

void ThreadYield( void ) {
    EA = 0;
    SAVESTATE ;
    for (;;) {
        if (ID == MAXTHREADS - 1) ID = 0;
        else ID++;
        // ID = ( ID == MAXTHREADS - 1 ) ? 0 : ID+1;
        if (threadBitmap[ID] > 0) break;
    }
    RESTORESTATE ;
    EA = 1;
}

void ThreadExit( void ) {
    EA = 0;
    threadBitmap[ID] = 0;
    for (;;) {
        if (ID == MAXTHREADS - 1) ID = 0;
        else ID++;
        // ID = ( ID == MAXTHREADS - 1 ) ? 0 : ID+1;
        if(threadBitmap[ID]>0) break;
    }
    RESTORESTATE;
    EA = 1;
}
