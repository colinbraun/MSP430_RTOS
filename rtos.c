/*
 * rtos.c
 *
 *  Created on: Mar 20, 2020
 *      Author: Colin
 */

#include <msp430.h>
#include <stdlib.h>
#include "rtos.h"

#define ENABLE_PINS 0xFFFE // Required to use inputs and outputs
#define SMCLK 0x0200 // Timer ACLK source
#define UP 0x0010 // Timer UP mode

void rtosSetup() {
	size = 0;
	currentProc = 0;
	PM5CTL0 = ENABLE_PINS; // Required to use inputs and outputs
	P1DIR |= BIT0;
	processes = (PCB*)malloc(10 * sizeof(PCB));
}

void rtosInitTask(void (*func)()) {
	PCB* process = (processes+size);
	process->id = size;
	process->function = func;
	size++;
}

/*
 * Run the currently loaded processes.
 *
 * Returns: 0 if all tasks completed, not 0 if an error occurred
 */
unsigned char rtosRun() {
	unsigned int id0 = (processes->id);
	unsigned int id1 = ((processes+1)->id);
	TA0CCR0 = 300; // Set up timer to generate interrupt every 30us
	TA0CTL = SMCLK | UP; // Set SMCLK, UP MODE
	TA0CCTL0 = CCIE; // Enable interrupt for Timer0
	_BIS_SR(GIE); // activate interrupts
	while (1)
		;
	return 0;
}

// Timer0 Interrupt Service Routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_ISR(void) {
	P1OUT ^= BIT0; // Toggle the red LED (for now, to test that this is working)
}
