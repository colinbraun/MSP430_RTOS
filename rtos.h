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

// A Process Control Block.
// Each one of these will be associated with a process
/*typedef struct PCB {
	uint16_t id;
	void (*function) (void);
} PCB;*/

// Global variable to hold the processes
PCBNode* processes;
// Hold the total number of processes in the list
unsigned char size;
// Hold the index of the current process
unsigned char currentProc;

void rtosInitTask(void (*func) (void));

void rtosSetup();

unsigned char rtosRun();

__interrupt void Timer0_ISR(void);

#endif /* RTOS_H_ */
