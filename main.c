#include <msp430.h> 
#include "rtos.h"
#include <driverlib.h> // Required for the LCD
#include "myGpio.h" // Required for the LCD
#include "myClocks.h" // Required for the LCD
#include "myLcd.h" // Required for the LCD
#include <stdlib.h>
#include <time.h>
#include <string.h>

void task2(void);
void task1(void);
void task3(void);
void scrollWords(char words[250]);

// Global variables
volatile unsigned char HoldGreenLED = 3;

/**
 * main.c
 */
void main(void) {
	WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
	initGPIO(); // Initialize General Purpose Inputs and Outputs for the LCD
	initClocks(); // Initialize clocks for the LCD
	myLCD_init(); // Initialize Liquid Crystal Display

	rtosSetup(); // Initialize the RTOS

	// task3 is started in task1 to prove a task can start a task while the RTOS is running
	rtosInitTask(&task1); // Initialize task1
	rtosInitTask(&task2); // Initialize task2

	rtosRun(); // Run the RTOS with the initialized tasks. Tasks can add tasks in the middle of execution.

	while (1)
		;
}

/*
 * task3(), every 10 seconds, assign a random value to HoldGreenLED and toggle the red LED (P1.0)
 */
void task3(void) {
	srand(time(NULL));
	TA3CCR0 = 40960; // setup TA2 timer to count for 10 seconds
	TA3CTL = 0x01D4; // start TA2 timer from zero in UP mode with ACLK and input divider by 8
	while (1) {
		if (TA3CTL & BIT0) {
			HoldGreenLED = rand() % 26; // Generate a random number for HoldGreenLED between 0 and 25
			TA3CTL &= ~BIT0; // reset timer flag
			P1OUT ^= BIT0; // Toggle the red LED
		}
		sleep(); // Waiting for timer, return control to OS early
	}
}

/*
 * task1() function. Every second, increment the number on the LCD.
 * If the user pushes P1.1, display "START OVER" and restart count to 0.
 */
void task1(void) {
	rtosInitTask(&task3); // Initialize task3, proving a task can start a task
	P1DIR &= ~BIT1; // Set P1.1 as an input
	unsigned int count = 0;
	TA1CCR0 = 32768; // setup TA2 timer to count for 1 second
	TA1CTL = 0x0114; // start TA2 timer from zero in UP mode with ACLK
	while (1) {
		if (TA1CTL & BIT0) // check if timer finished counting
		{
			TA1CTL &= ~BIT0; // reset timer flag
			count++; // increment counter
			myLCD_displayNumber(count);
		}
		if (!(P1IN & BIT1)) {
			scrollWords("START OVER");
			count = 0;
		}
		sleep(); // Waiting for timer or user input, so sleep
	}
}

//************************************************************************
// void Task2(void) Function
//
// Uses Timer TA2 to create a 1 HZ, 50% duty cycle
// signal on P9.7 (Green LED) and also every three
// seconds checks a global variable (HoldGreenLED)
// and will hold P9.7 high for the number of seconds
// corresponding to the interger contained in HoldGreenLED.
// Afterward, the 1 Hz 50% duty cycle signal resumes.
//************************************************************************
//
// Phil Walter
// 03/18/2020  v1
// for CE-422, Project 5
//************************************************************************
void task2(void) {
// Setup - runs once
	unsigned char count = 0;   // local variable

	P9DIR |= BIT7;       // make P9.7 (Green LED) an output
	P9OUT &= ~BIT7;      // Turn off P9.7 (Green LED)

	TA2CCR0 = 16384;     // setup TA2 timer to count for 0.5 second
	TA2CTL = 0x0114;     // start TA2 timer from zero in UP mode with ACLK

// loop that repeats forever - 1Hz 50% DC signl or on for HoldGreenLED seconds
	while (1)             // infinite loop
	{
// 1 Hz 50% DC signal
		if (TA2CTL & BIT0)     // check if timer finished counting
		{
			TA2CTL &= ~BIT0;     // reset timer flag
			P9OUT ^= BIT7;       // toggle P9.7 (Green LED)
			count++;             // increment counter
		}             //end if (TA2CTL & BIT0)

// every three seconds get HoldGreenLED value and make P9.7 high
		if (count >= 6)        // 3 seconds elapsed?
				{
			TA2CCR0 = 32768;     // set timer to count for 1 second
			TA2CTL = 0x0114;   // start TA2 timer from zero in UP mode with ACLK
			P9OUT |= BIT7;       // Turn On P9.7 (Green LED)

			count = HoldGreenLED; // set local variable equal to global variable
			// how many seconds to keep P9.7 high
			while (count > 0)         // count down to zero
			{
				if (TA2CTL & BIT0)   // check if timer done counting to 1 second
				{
					TA2CTL &= ~BIT0;      // reset timer flag
					count--;              // decrement counter
				}
			}              //end while(count > 0)

// Done with P9.7 high, go back to 1 Hz 50% DC signal
			TA2CCR0 = 16384;      // setup TA2 timer to count for 0.5 second
			TA2CTL = 0x0114;   // start TA2 timer from zero in UP mode with ACLK
			P9OUT ^= BIT7;        // start with P9.7 (Green LED) off

		}        //end if (count >= 6)

	}        //end while(1)

}        //end Task2()

/*
 * Function to scroll some words across the LCD
 */
void scrollWords(char words[250]) {
	unsigned int length; // Contains length of message to be displayed
	unsigned int slot; // Slot to be displayed on LCD (1, 2, 3, 4,
	// 5, or 6)
	unsigned int amount_shifted; // Number of times message shifted so far
	unsigned int offset; // Used with amount_shifted to get correct
	// character to display
	unsigned long delay; // Used to implement delay between scrolling
	// iterations
	unsigned char next_char; // Next character from message to be
	// displayed
	length = strlen(words); // Get length of the message stored in words
	amount_shifted = 0; // We have not shifted the message yet
	offset = 0; // There is no offset yet
	while (amount_shifted < length + 7) // Loop as long as you haven't shifted all
	{ // of the characters off the LCD screen
		offset = amount_shifted; // Starting point in message for next LCD update
		for (slot = 1; slot <= 6; slot++) // Loop 6 times to display 6 characters at a time
				{
			next_char = words[offset - 6]; // Get the current character for LCD slot
			if (next_char && (offset >= 6) && (offset <= length + 6)) // If character is not null AND
					{ // LCD is not filled (offset>=6) AND
					  // You have not reached end of message
					  // (offset<=length+6)
				myLCD_showChar(next_char, slot); // Show the next character on the LCD
				// screen in correct slot
			}  //end if
			else // Else, slot on LCD should be blank
			{
				myLCD_showChar(' ', slot); // So, add a blank space to slot
			} //end else
			offset++; // Update as you move across the message

		} //end for
		for (delay = 0; delay < 75000/*123456*/; delay = delay + 1)
			; // Delay between shifts
		amount_shifted = amount_shifted + 1; // Update times words shifted across LCD
	} //end while
}
