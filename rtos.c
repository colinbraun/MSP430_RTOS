/*
 * rtos.c
 *
 *  Created on: Mar 20, 2020
 *      Author: Colin
 */

#include "rtos.h"

void rtosSetup() {
	size = 0;
	currentProc = 0;
}

void rtosInitTask(void (*func) ()) {
	PCB process = processes[size];
	process.id = size;
	process.function = func;
}

/*
 * Run the currently loaded processes.
 *
 * Returns: 0 if all tasks completed, not 0 if an error occurred
 */
unsigned char rtosRun() {
	unsigned int id = processes[0].id;
	(*processes[0].function)();
	return 0;
}
