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

void loadProcData(PCB* pcb);
// A Process Control Block.
// Each one of these will be associated with a process
/*typedef struct PCB {
	uint16_t id;
	void (*function) (void);
} PCB;*/

// Global variable to hold the processes
PCBNode* processes;
PCB pcb_test;
// Hold the total number of processes in the list
unsigned char size;
// Hold the index of the current process
unsigned char currentProc;

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
