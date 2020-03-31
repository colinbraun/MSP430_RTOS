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
// Hold the total number of processes in the list
volatile unsigned char size;
// Hold the index of the current process
volatile unsigned char currentProc;

volatile uint16_t *oldStackPointer;
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
 * Run the currently loaded processes.
 *
 * Returns: 0 if all tasks completed, not 0 if an error occurred
 */
unsigned char rtosRun();

__interrupt void Timer0_ISR(void);

#endif /* RTOS_H_ */
