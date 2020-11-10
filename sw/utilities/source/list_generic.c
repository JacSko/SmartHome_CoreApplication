#include "list_generic.h"

#include <stdlib.h>


const uint8_t DEFAULT_LIST_SIZE = 20;

LIST_ITEM* list_items;

L_LIST* list_init()
{
	L_LIST* list = (L_LIST *) malloc (sizeof(L_LIST));
	if (list)
	{
		list->head = NULL;
		list->size = 0;
	}
	return list;

}
void list_deinit(L_LIST* list)
{
	if (!list) return;
	LIST_ITEM* item = list->head;
	if (list->size > 0)
	{
		while(item->next != NULL)
		{
			free(item->data);
			item = item->next;
			free (item->prev);
		}
	}

	free(list);

}
RET_CODE list_add(L_LIST* list, void* data)
{
	RET_CODE result = RETURN_NOK;
	if (list->size < 255)
	{
		LIST_ITEM* item = (LIST_ITEM*) malloc (sizeof(LIST_ITEM));
		if (item)
		{
			item->data = data;
			item->next = NULL;

			LIST_ITEM* last_element = list->head;
			if (!last_element)
			{
				list->head = item;
			}
			else
			{
				while (last_element->next != NULL)
				{
					last_element = last_element->next;
				}
				last_element->next = item;
				item->prev = last_element;
			}
			list->size++;
			result = RETURN_OK;
		}
	}
	return result;
}
RET_CODE list_remove(L_LIST* list, void* data)
{
	RET_CODE result = RETURN_NOK;

	if (list->size > 0)
	{
		LIST_ITEM* last_element = list->head;
		while (last_element)
		{
			if (last_element->data == data)
			{
				if (last_element == list->head)
				{
					list->head = list->head->next;
				}
				if (last_element->prev) last_element->prev->next = last_element->next;
				if (last_element->next) last_element->next->prev = last_element->prev;

				free(last_element);
				result = RETURN_OK;
				list->size--;
				if (list->size == 0)
				{
					list->head = NULL;
				}
				break;
			}
			else
			{
				last_element = last_element->next;
			}
		}
	}

	return result;

}
uint8_t list_get_size(L_LIST* list)
{
	return list->size;
}

