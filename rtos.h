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
#include "linked_list.h"

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

// A Process Control Block.
// Each one of these will be associated with a process
/*typedef struct PCB {
	uint16_t id;
	void (*function) (void);
} PCB;*/

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

volatile char procEnded;
/*
 * Add the passed function as a process that will be given time slices when rtosRun() is invoked
 */
void rtosInitTask(void (*func) (void));

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
void removeProc(uint8_t id);

/*
* Find the next process id from the current one
*/
uint8_t findNextProc();

/*
 * Run the RTOS with the already loaded tasks/processes
 */
unsigned char rtosRun();

__interrupt void Timer0_ISR(void);

#endif /* RTOS_H_ */
