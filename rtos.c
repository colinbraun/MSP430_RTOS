/*
 * rtos.c
 *
 *  Created on: Mar 20, 2020
 *      Author: Colin
 */

#include "rtos.h"
#include "linked_list.h"

#define ENABLE_PINS 0xFFFE // Required to use inputs and outputs
#define SMCLK 0x0200 // Timer ACLK source
#define UP 0x0010 // Timer UP mode

void rtosSetup() {
	size = 0;
	currentProc = 0;
	PM5CTL0 = ENABLE_PINS; // Required to use inputs and outputs
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;
	//processes = NULL;
}

void rtosInitTask(void (*func)()) {
	processes[size].id = size;
	processes[size].function = func;
	processes[size].stackPointer = &processes[size].ram[PROCESS_RAM - 1];

	// Load the process's function as the return address. Still need the other 4 bits of the 20-bit address
	*processes[size].stackPointer = (uint16_t)func;
	*processes[size].stackPointer--;
	// Now load the upper-most 4 bits of the 20-bit return address into the upper-most 16 bits of the next word.
	*processes[size].stackPointer = (uint16_t)(((0xF0000 & (uint32_t)func) >> 4) | 0x00); // Bitwise or with initial SR settings

	// Make room for general purpose registers
	*processes[size].stackPointer -= NUM_GEN_REGS * 2; // *2 because we are using their full 20 bits

	size++;
}

unsigned char rtosRun() {
	//unsigned int id0 = processes->data.id;
	//unsigned int id1 = processes->next->data.id;
	TA0CCR0 = 300; // Set up timer to generate interrupt every 300us
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

	// P1OUT ^= BIT0; // Toggle the red LED (for now, to test that this is working)
	// __asm("	XOR.B #1, &0x202"); // test some asm code to toggle the red led (this works)
	//size_t addr = &pcb_test;
	//size_t another_val = &rtosRun;
	//loadProcData(&pcb_test);
	//PCB result = pcb_test;
	// Do all the task managing
}
