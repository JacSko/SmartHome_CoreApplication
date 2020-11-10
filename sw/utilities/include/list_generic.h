#ifndef _LIST_GENERIC_H_
#define _LIST_GENERIC_H_

#include <stdint.h>
#include "return_codes.h"

/**
 * 	Implementation of one-directional linked list.
 * 	It allows dynamic allocation of objects.
 * 	List size is restricted to 255.
 *
 * 	USAGE: list_init to get the pointer to new list instance, than use all methods.
 */

typedef struct LIST_ITEM
{
	void* data;
	struct LIST_ITEM* next;
	struct LIST_ITEM* prev;
} LIST_ITEM;

typedef struct L_LIST
{
	uint8_t size;
	LIST_ITEM* head;
} L_LIST;

L_LIST* list_init();
void list_deinit(L_LIST*);
RET_CODE list_add(L_LIST*, void*);
RET_CODE list_remove(L_LIST*, void*);
uint8_t list_get_size(L_LIST*);






#endif
