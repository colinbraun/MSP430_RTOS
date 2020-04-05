/*
 * rtos.c
 *
 *  Created on: Mar 20, 2020
 *      Author: Colin
 */

#include "rtos.h"

#define ENABLE_PINS 0xFFFE // Required to use inputs and outputs
#define SMCLK 0x0200 // Timer ACLK source
#define UP 0x0010 // Timer UP mode

/*
 * Set up the rtos.
 * Should be called before any other rtos function.
 */
void rtosSetup() {
	rtosStarted = 0;
	availableProcs = 0;
	currentProc = 0;
	procEnded = 0;
	PM5CTL0 = ENABLE_PINS; // Required to use inputs and outputs
	P1DIR |= BIT0;
	P1OUT &= ~BIT0;
	uint8_t i;
	// Initialize all the process ids
	for (i = 0; i < MAX_PROCS; i++) {
		processes[i].id = i;
	}
}

/*
 * Add the passed function as a process that will be given time slices when rtosRun() is invoked
 *
 * param func - A pointer to the function (task) to be initialized
 */
void rtosInitTask(void (*func)()) {
	// CRITICAL SECTION
	_BIC_SR(GIE); // Disable interrupts while adding a task
	uint8_t i = 0;
	while (availableProcs & (1 << i))
		i++;
	availableProcs |= (1 << i);

	processes[i].function = func;
	processes[i].stackPointer = &processes[i].ram[PROCESS_RAM - 1]; // Start with the stack pointer ready to load the first word

	// Create the return address for process
	processes[i].stackPointer--; // Move the stack pointer down an extra word (need 32 bits for return address, already have 16)
	*((uint32_t*) processes[i].stackPointer) = (uint32_t) (&processTerminate); // Store the return address on the process's stack

	processes[i].stackPointer--; // Move stack pointer down 1 word to store first lower 16 bits of return address from RETI
	// Load the process's function as the return address. Still need the other 4 bits of the 20-bit address
	*processes[i].stackPointer = (uint16_t) func;
	processes[i].stackPointer--; // Move the stack pointer down 1 more word
	// Now load the upper-most 4 bits of the 20-bit return address into the upper-most 16 bits of the next word. Also set up SR bits.
	*processes[i].stackPointer = (((0xF0000 & (uint32_t) func) >> 4) | GIE); // Bitwise or with initial SR settings (enable interrupts)

	// Make room for general purpose registers (using them as 16-bit registers, not 20-bit)
	processes[i].stackPointer -= NUM_GEN_REGS;
	if(rtosStarted)
		_BIS_SR(GIE); // If the RTOS has started, re-enable interrupts
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
		asm volatile ("\tmov.w #0, R12");
		// Move the stack pointer back to where it was right after rtosRun was called (so the stack pointer has the correct return address)
		asm volatile ("\tmovx.a rtosStackPointer, R1");
		return;
	}
	// The next process will automatically be selected by the timer interrupt

	// Re-enable interrupts
	_BIS_SR(GIE);
	// Return control to the operating system. Not an issue if in the middle of calling this an interrupt happens, we won't return here anyway
	sleep();
}

/*
 * Remove a process from the pool of currently running ones
 * NOTE: It is the caller of this function's job to be concerned with an interrupt happening during this function.
 * It is likely (not tested) that this function is thread-safe, since a 16-bit assignment is usually atomic.
 * However, computing the value to be stored may take several instructions, if that is of any concern.
 *
 * param id - the id of the process to remove
 */
inline void removeProc(uint8_t id) {
	availableProcs &= ~(1 << id);
}

/*
 * Run the RTOS with the already loaded tasks/processes
 */
unsigned char rtosRun() {
	// Store the stack pointer at this point (will need it when there are no more processes)
	// note, when restoring this, whatever is in R12 will be the return value.
	asm volatile ("\tmovx.a R1, rtosStackPointer");
	oldStackPointer = processes[0].stackPointer; // Prepare to load the stack pointer of the first process
	// Set up timer to generate interrupt every 900us. Too short and OS takes a lot of time. Too long and the responsiveness of tasks will be poor.
	// The faster the OS switches context, the lower we can justify this to be.
	TA0CCR0 = 900;
	TA0CTL = SMCLK | UP; // Set SMCLK, UP MODE
	TA0CCTL0 = CCIE; // Enable interrupt for Timer0
	rtosStarted = 1; // RTOS has started
	asm volatile ("\tmovx.a oldStackPointer, R1");
	LOAD_CONTEXT();
	// This will enable interrupts

	// This will never happen. The actual return of this function takes place in processTerminate()
	return 0;
}

/*
 * Find the next process id from the current one.
 * Wish to maybe improve this through a different appraoch to process management, since it takes a lot of computing time to switch tasks
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
 * Function to call to tell the OS to switch to another task for now.
 * This is intended to be called by tasks that wish to end their current time slice and wait for their next
 */
inline void sleep() {
	TA0CCTL0 |= CCIFG;
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

	asm volatile ("\tPOPM.A #5, R15");
	// The interrupt does PUSHM.A #5, R15 at the beginning. This was causing a headache. Temporary fix for now.
	// If we aren't coming from a process ending, then...
	if (!procEnded) {
		// Store the context onto the stack.
		// Don't need to store the PC or SR, since they are already on THIS stack from the interrupt happening.
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
