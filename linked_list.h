/*
 * A linked list structure to be used by other programs
 */

#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stdlib.h>
#include <stdint.h>

//typedef struct PCB PCB;
typedef struct PCB {
	uint16_t id;
	void (*function) (void);
} PCB;

typedef struct Node {
	PCB data; // The data that this Node holds
	struct Node *next; // The next Node in the list
} PCBNode;

/*
 * Add an element to the head of the list
 * If a pointer to a null pointer is passed, the pointer to a null pointer will now point to a pointer to the head node
 * In other words: To create a list initially, create a null pointer of type PCBNode and pass a reference to it.
 */
void addToListHead(PCBNode** head, PCB data);

/*
 * Add an element to the end of the list
 */
void addToListEnd(PCBNode** head, PCB data);

/*
 * Clean up the passed list (deallocate the memory)
 */
void destroyList(PCBNode** head);

#endif
