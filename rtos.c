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
	currentProc = 0;
	procEnded = 0;
	PM5CTL0 = ENABLE_PINS; // Required to use inputs and outputs
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;
	uint8_t i;
	// Initialize all the process ids
	for(i = 0; i < MAX_PROCS; i++) {
		processes[i].id = i;
	}
}

void rtosInitTask(void (*func)()) {
	uint8_t i = 0;
	while(availableProcs & (1 << i))
		i++;
	availableProcs |= (1 << i);
	processes[i].function = func;
	uint32_t addr = (uint32_t) func;
	processes[i].stackPointer = &processes[i].ram[PROCESS_RAM - 1];
	addr = &processes[i].ram[PROCESS_RAM - 1];

	// Create the return address for process
	processes[i].stackPointer--;
	*((uint32_t*) processes[i].stackPointer) =
			(uint32_t) (&processTerminate);

	uint32_t addr2 = &processTerminate;
	processes[i].stackPointer--;

	// Load the process's function as the return address. Still need the other 4 bits of the 20-bit address
	*processes[i].stackPointer = (uint16_t) func;
	processes[i].stackPointer--;
	// Now load the upper-most 4 bits of the 20-bit return address into the upper-most 16 bits of the next word.
	*processes[i].stackPointer = (((0xF0000 & (uint32_t) func) >> 4) | GIE); // Bitwise or with initial SR settings (enable interrupts)

	// Make room for general purpose registers
	processes[i].stackPointer -= NUM_GEN_REGS;
}

/*
 * Called automatically when a process terminates.
 */
static void processTerminate() {
	// CRITICAL SECTION
	// First disable interrupts. We are not bothered if an interrupt occurs immediately after we enter this function.
	_BIC_SR(GIE);
	procEnded = 1;
	removeProc(currentProc);
	// If there are no more processes
	if (!availableProcs) {
		// Set the return value to be 0
		asm volatile ("\tmov.w #0, R0");
		// Move the stack pointer back to where it was right after rtosRun was called (so the stack pointer has the correct return address)
		asm volatile ("\tmovx.a R1, rtosStackPointer");
		return;
	}
	// The next process will automatically be selected by the timer interrupt

	// Re-enable interrupts
	_BIS_SR(GIE);
	// Just wait for the interrupt to happen. This will never be returned to.
	while (1) {

	}
}

/*
 * Remove a process from the pool of currently running ones
 * NOTE: It is the caller of this function's job to be concerned with an interrupt happening during this function.
 * It is likely (not tested) that this function is thread-safe, since a 16-bit assignment is usually atomic.
 * However, computing the value to be stored may take several instructions, if that is of any concern.
 *
 * param id - the id of the process to remove
 */
void removeProc(uint8_t id) {
	availableProcs &= ~(1 << id);
}

/*
 * Run the RTOS with the already loaded tasks/processes
 */
unsigned char rtosRun() {
	// Store the stack pointer at this point (will need it when there are no more processes)
	// note, when restoring this, whatever is in R12 will be the return value.
	asm volatile ("\tmovx.a R1, rtosStackPointer");
	//unsigned int id0 = processes->data.id;
	//unsigned int id1 = processes->next->data.id;
	oldStackPointer = processes[0].stackPointer;
	TA0CCR0 = 300; // Set up timer to generate interrupt every 300us
	TA0CTL = SMCLK | UP; // Set SMCLK, UP MODE
	TA0CCTL0 = CCIE; // Enable interrupt for Timer0
	//uint32_t *addr = oldStackPointer;
	//uint32_t addr_addr = &oldStackPointer;
	asm volatile ("\tmovx.a oldStackPointer, R1");
	LOAD_CONTEXT();
	// This will enable interrupts
	while (1)
		;
	// This will never happen. The actual return of this function takes place in processTerminate()
	return 0;
}

/*
 * Find the next process id from the current one
 */
inline uint8_t findNextProc() {
	uint8_t nextProc = currentProc + 1;
	while (!(availableProcs & (1 << nextProc)) && nextProc != currentProc) {
		if (nextProc < MAX_PROCS - 1)
			nextProc++;
		else
			nextProc = 0;
	}
	return nextProc;
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
	asm volatile ("\tPOPM.A #5, R15");
	// If we aren't coming from a process ending, then...
	if (!procEnded) {
		// The interrupt does PUSHM.A #5, R15 at the beginning. This was causing a headache. Temporary fix for now.
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
	}
	currentProc = findNextProc();
	procEnded = 0;
	oldStackPointer = processes[currentProc].stackPointer;
	// Load the stack pointer for the next process
	asm volatile ("\tmovx.a oldStackPointer, R1");

	// Load the context from the newly loaded stack
	LOAD_CONTEXT();
}
