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
	//uint32_t addr = (uint32_t)func;
	processes[size].stackPointer = &processes[size].ram[PROCESS_RAM - 1];
	//addr = &processes[size].ram[PROCESS_RAM - 1];

	// Load the process's function as the return address. Still need the other 4 bits of the 20-bit address
	*processes[size].stackPointer = (uint16_t) func;
	processes[size].stackPointer--;
	// Now load the upper-most 4 bits of the 20-bit return address into the upper-most 16 bits of the next word.
	*processes[size].stackPointer = (((0xF0000 & (uint32_t) func)
			>> 4) | GIE); // Bitwise or with initial SR settings (enable interrupts)

	// Make room for general purpose registers
	processes[size].stackPointer -= NUM_GEN_REGS;

	size++;
}

unsigned char rtosRun() {
	//unsigned int id0 = processes->data.id;
	//unsigned int id1 = processes->next->data.id;
	oldStackPointer = processes[0].stackPointer;
	TA0CCR0 = 300; // Set up timer to generate interrupt every 300us
	TA0CTL = SMCLK | UP; // Set SMCLK, UP MODE
	TA0CCTL0 = CCIE; // Enable interrupt for Timer0
	uint32_t *addr = oldStackPointer;
	uint32_t addr_addr = &oldStackPointer;
	asm volatile ("\tmovx.a oldStackPointer, R1");
	LOAD_CONTEXT(); // This will enable interrupts
	while (1)
		;
	return 0;
}

/*
 * NOTE <----------------------------------------------------------------------------
 * DO NOT DECLARE VARIABLES IN THIS FUNCTION.
 * THEY ARE PUT ONTO THE STACK.
 * WE DON'T WANT TO DEAL WITH THAT.
 */
// Timer0 Interrupt Service Routine - The scheduler
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_ISR(void) {
	//P1OUT ^= BIT0; // Toggle the red LED (for now, to test that this is working)
	// Store the context onto the stack.
	// Don't need to store the PC or SR, since they are already on THIS stack from the interrupt happening.
	asm volatile ("\tPOPM.A #5, R15"); // The interrupt does PUSHM.A #5, R15 at the beginning. This was causing a headache. Temporary fix for now.
	asm volatile ("\tpush r4  \n\t push r5  \n\t\
	                 push r6  \n\t\
	                 push r7  \n\t\
	                 push r8  \n\t\
	                 push r9  \n\t\
	                 push r10 \n\t\
	                 push r11 \n\t\
	                 push r12 \n\t\
	                 push r13 \n\t\
	                 push r14 \n\t\
	                 push r15");
	asm volatile ("\tmovx.a R1, oldStackPointer");
	// Store the stack pointer for the previously running process
	processes[currentProc].stackPointer = oldStackPointer;
	if (currentProc < size - 1)
		currentProc++;
	else
		currentProc = 0;
	oldStackPointer = processes[currentProc].stackPointer;
	// Load the stack pointer for the next process
	asm volatile ("\tmovx.a oldStackPointer, R1");

	// Load the context from the newly loaded stack
	LOAD_CONTEXT();
	// __asm("	XOR.B #1, &0x202"); // test some asm code to toggle the red led (this works)
	//size_t addr = &pcb_test;
	//size_t another_val = &rtosRun;
	//loadProcData(&pcb_test);
	//PCB result = pcb_test;
	// Do all the task managing
}
