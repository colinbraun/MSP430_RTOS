/*
 * rtos.h
 *
 *  Created on: Mar 20, 2020
 *      Author: Colin
 */

#ifndef RTOS_H_
#define RTOS_H_

#include <stdint.h>
#include <msp430.h>
#include <stdlib.h>

#define MAX_PROCS 3 // The maximum number of processes that can held at once
#define NUM_GEN_REGS 12 // The number of general purpose registers (R4-R15)
#define PROCESS_RAM 256 // Each process will have 256 words of ram
#define LOAD_CONTEXT() asm volatile ( "\tpop r15 \n\t \
	                 pop r14 \n\t\
	                 pop r13 \n\t\
	                 pop r12 \n\t\
	                 pop r11 \n\t\
	                 pop r10 \n\t\
	                 pop r9  \n\t\
	                 pop r8  \n\t\
	                 pop r7  \n\t\
	                 pop r6  \n\t\
	                 pop r5  \n\t\
	                 pop r4  \n\t\
	                 reti"\
	               )

// The process control block struct
typedef struct PCB {
	uint8_t id; // The process id. This should map directly to the index of the array it's stored in
	void (*function) (void); // The function itself
	volatile uint16_t *stackPointer; // Where the process's stack pointer points to
	uint16_t ram[PROCESS_RAM]; // The process's ram
} PCB;

// Global variable to hold the processes. Better method would be to dynamically allocate memory as needed.
PCB processes[MAX_PROCS];

// Hold the index (and id, inherently) of the current process
volatile uint8_t currentProc;

// A placeholder for a stack pointer (when switching out stacks)
volatile uint16_t *oldStackPointer;

// Hold a pointer to the stack for right after rtosRun() is called.
volatile uint16_t *rtosStackPointer;

// Each bit represents a process, 0 = free, 1 = used
volatile uint16_t availableProcs;

// If a process just ended, scheduler will need to behave differently. It checks this.
volatile uint8_t procEnded;

volatile uint8_t rtosStarted;

/*
 * Add the passed function as a process that will be given time slices when rtosRun() is invoked
 *
 * param func - A pointer to the function (task) to be initialized
 */
void rtosInitTask(void (*func)(void));

/*
 * Set up the rtos.
 * Should be called before any other rtos function.
 */
void rtosSetup();

/*
 * Called automatically when a process terminates.
 */
static void processTerminate();

/*
 * Remove a process from the pool of currently running ones
 * NOTE: It is the caller of this function's job to be concerned with an interrupt happening during this function.
 * It is likely (not tested) that this function is thread-safe, since a 16-bit assignment is usually atomic.
 * However, computing the value to be stored may take several instructions, if that is of any concern.
 *
 * param id - the id of the process to remove
 */
inline void removeProc(uint8_t id);

/*
 * Find the next process id from the current one
 */
inline uint8_t findNextProc();

/*
 * Function to call to tell the OS to switch to another task for now.
 * This is intended to be called by tasks that wish to end their current time slice and wait for their next.
 * Inlining this was not working for some reason, just just changed it to a #define
 */
#define sleep() TA0CCTL0 |= CCIFG

/*
 * Run the currently loaded processes.
 *
 * return - 0 if all tasks completed, not 0 if an error occurred
 */
unsigned char rtosRun();

__interrupt void Timer0_ISR(void);

#endif /* RTOS_H_ */
