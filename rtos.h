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

#define PROCESS_RAM 256 // Each process will have 512 words of ram
// The process control block struct
typedef struct PCB {
	uint8_t id; // The process id. This should map directly to the index of the array it's stored in
	void (*function) (void); // The function itself
	volatile uint16_t *stackPointer; // Where the process's stack pointer points to
	uint16_t ram[PROCESS_RAM]; // The processes' ram
} PCB;

// Global variable to hold the processes
PCB processes[MAX_PROCS];
//PCBNode* processes;
//PCB pcb_test;

// Hold the index (and id, inherently) of the current process
volatile uint8_t currentProc;

// A placeholder for a stack pointer (when switching out stacks)
volatile uint16_t *oldStackPointer;

// Hold a pointer to the stack for right after rtosRun() is called.
volatile uint16_t *rtosStackPointer;

// Each bit represents a process, 0 = free, 1 = used
volatile uint16_t availableProcs;

// If a process just ended, scheduler will need to behave differently. It checks this.
volatile char procEnded;

/*
 * Add the passed function as a process that will be given time slices when rtosRun() is invoked
 */
void rtosInitTask(void (*func)(void));

/*
 * Set up the rtos.
 * Should be called before any other rtos function.
 */
void rtosSetup();

/*
 * Function that is returned to when a process terminates
 */
static void processTerminate();
/*
 * Run the currently loaded processes.
 *
 * Returns: 0 if all tasks completed, not 0 if an error occurred
 */

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
 * This is intended to be called by tasks that wish to end their current time slice and wait for their next
 */
void sleep();

/*
 * Run the RTOS with the already loaded tasks/processes
 */
unsigned char rtosRun();

__interrupt void Timer0_ISR(void);

#endif /* RTOS_H_ */
