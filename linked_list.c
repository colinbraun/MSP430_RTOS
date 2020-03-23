/*
 * linked_list.c
 *
 *  Created on: Mar 21, 2020
 *      Author: Colin
 */

#include "linked_list.h"

void addToListHead(PCBNode** head, PCB data) {
	PCBNode* node = (PCBNode*) malloc(sizeof(PCBNode));
	node->next = *head;
	node->data = data;
	(*head) = node;
}

void destroyList(PCBNode** head) {
	if(head == 0)
		return;
	PCBNode* next = *head;
	while (next) {
		PCBNode* temp = next->next;
		free(next);
		next = temp;
	}
}
