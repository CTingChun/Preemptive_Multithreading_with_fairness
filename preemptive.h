/*
 * file: preemptive.h
 *
 * this is the include file for the preemptive multithreading
 * package.  It is to be compiled by SDCC and targets the EdSim51 as
 * the target architecture.
 *
 * CS 3423 Fall 2018
 */

#ifndef __PREEMPTIVE_H__
#define __PREEMPTIVE_H__

#define CNAME(s) _ ## s
#define LABNAME(lab) lab ## $

typedef char ThreadID;
typedef void (*FunctionPtr)(void);

ThreadID ThreadCreate(FunctionPtr);
void ThreadYield(void);
void ThreadExit(void);

#define SemaphoreWaitBody(S, label) \
{ __asm \
    LABNAME(label): \
    mov ACC, CNAME(S)\
    JB ACC.7, LABNAME(label)\
    dec  CNAME(S) \
__endasm; }
#define SemaphoreCreate(s, n) s = n;
// create a counting semaphore s initialized to n
#define SemaphoreWait(s) SemaphoreWaitBody(s, __COUNTER__);
// do busy-wait on semaphore s
#define SemaphoreSignal(s) {__asm INC CNAME(s) __endasm;}
// signal a semaphore s
#endif // __COOPERATIVE_H__

