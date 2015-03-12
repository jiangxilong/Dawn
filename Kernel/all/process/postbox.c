/*
 * postbox.c
 *
 *  Created on: 15 Oct 2014
 *      Author: blake
 */

#include <process/postbox.h>
#include <stdlib.h>

process_message* postboxTop(process_postbox* pb, process_message* dest) {

	if (!pb->first) {
		return 0;
	}

	postbox_message_entry* message = pb->first;

	//Update the linked list head
	if (pb->first->next != 0) {
		pb->first = message->next;
	} else {
		pb->first = 0;
	}

	*dest = message->data;
	free(message);

	return dest;
}

process_message postboxPeek(process_postbox* pb, process_message* dest) {

	if (!pb->first) {
		return 0;
	}

	*dest = pb->first->data;
	return dest;
}

void postboxPush(process_postbox* pb, process_message msg) {

	if (pb->first == 0) {
		//Create pb->first
		postbox_message_entry* new_entry = malloc(
				sizeof(postbox_message_entry));
		new_entry->data = msg;
		new_entry->next = 0;

		pb->first = new_entry;
	} else {
		//Add to the end of the list
		postbox_message_entry* last = pb->first;

		//Find the last entry
		while (1) {
			if (last->next == 0) {
				break;
			} else {
				last = last->next;
			}
		}

		postbox_message_entry* new_entry = malloc(
				sizeof(postbox_message_entry));
		new_entry->data = msg;
		new_entry->next = 0;

		last->next = new_entry;
	}
}

