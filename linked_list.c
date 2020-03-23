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

void addToListEnd(PCBNode** head, PCB data) {
	if (!head)
		return;
	PCBNode* next = *head;
	if (!next) {
		addToListHead(head, data);
		return;
	}
	while (next->next) {
		next = next->next;
	}
	PCBNode* new_node = (PCBNode*) malloc(sizeof(PCBNode));
	new_node->next = NULL;
	new_node->data = data;
	next->next = new_node;
}

void destroyList(PCBNode** head) {
	if (!head)
		return;
	PCBNode* next = *head;
	while (next) {
		PCBNode* temp = next->next;
		free(next);
		next = temp;
	}
}
