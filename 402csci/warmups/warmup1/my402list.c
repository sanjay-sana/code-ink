#include <stdio.h>
#include <stdlib.h>
#include "my402list.h"

int  My402ListLength(My402List* linkedList)
{
	return linkedList->num_members;
}

int  My402ListEmpty(My402List* linkedList)
{
	if (linkedList->num_members == 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

My402ListElem *My402ListFirst(My402List* linkedList)
{
	if (My402ListEmpty(linkedList))
	{
		return NULL;
	}
	else
	{
		return (linkedList->anchor).next;
	}
}

My402ListElem *My402ListLast(My402List* linkedList)
{
	if (My402ListEmpty(linkedList))
	{
		return NULL;
	}
	else
	{
		return (linkedList->anchor).prev;
	}
}

My402ListElem *My402ListPrev(My402List* linkedList, My402ListElem* element)
{
	if (element == My402ListFirst(linkedList))
	{
		return NULL;
	}
	else
	{
		return element->prev;
	}
}

My402ListElem *My402ListNext(My402List* linkedList, My402ListElem* element)
{
	if (element == My402ListLast(linkedList))
	{
		return NULL;
	}
	else
	{
		return element->next;
	}
}

My402ListElem *My402ListFind(My402List* linkedList, void* object)
{
	if (!My402ListEmpty(linkedList))
	{
		My402ListElem *tempElement;
		tempElement = My402ListFirst(linkedList);
		for (int i = 0; i < linkedList->num_members; i++)
		{
			if (tempElement->obj == object)
			{
				return tempElement;
			}
			tempElement = My402ListNext(linkedList, tempElement);
			// tempElement = tempElement->next;
		}
		return NULL;
	}
	else
	{
		return NULL;
	}
}

int  My402ListAppend(My402List* linkedList, void* object)
{
	My402ListElem *newElement;
	newElement = (My402ListElem*)malloc(sizeof(My402ListElem));
	if (!newElement)
	{
		return FALSE;
	}
	newElement->obj = object;
	if (linkedList->num_members == 0)
	{
		newElement->prev = &(linkedList->anchor);
		newElement->next = &(linkedList->anchor);
		(linkedList->anchor).prev = newElement;
		(linkedList->anchor).next = newElement;
	}
	else
	{
		My402ListElem *lastElement;
		lastElement = My402ListLast(linkedList);
		if (lastElement != NULL)
		{
			newElement->next = &(linkedList->anchor);
			newElement->prev = lastElement;
			lastElement->next = newElement;
			(linkedList->anchor).prev = newElement;
		}
	}
	linkedList->num_members++;
	return TRUE;
}

int  My402ListPrepend(My402List* linkedList, void* object)
{
	My402ListElem *newElement;
	newElement = (My402ListElem*)malloc(sizeof(My402ListElem));
	if (!newElement)
	{
		return FALSE;
	}
	newElement->obj = object;
	if (linkedList->num_members == 0)
	{
		newElement->prev = &(linkedList->anchor);
		newElement->next = &(linkedList->anchor);
		(linkedList->anchor).prev = newElement;
		(linkedList->anchor).next = newElement;
	}
	else
	{
		My402ListElem *firstElement;
		firstElement = My402ListFirst(linkedList);
		if (firstElement != NULL)
		{
			newElement->next = firstElement;
			newElement->prev = &(linkedList->anchor);
			firstElement->prev = newElement;
			(linkedList->anchor).next = newElement;
		}
	}
	linkedList->num_members++;
	return TRUE;
}

void My402ListUnlink(My402List* linkedList, My402ListElem* element)
{
	if (!My402ListEmpty(linkedList))
	{
		My402ListElem *prevElement, *nextElement;
		// prevElement = My402ListPrev(linkedList, element);
		// nextElement = My402ListNext(linkedList, element);
		prevElement = element->prev;
		nextElement = element->next; 
		nextElement->prev = element->prev;
		prevElement->next = element->next;
		free(element);
		linkedList->num_members--;
	}
}

void My402ListUnlinkAll(My402List* linkedList)
{
	My402ListElem *currentElement, *temp;
	currentElement = My402ListFirst(linkedList);
	while (!My402ListEmpty(linkedList))
	{
		temp = currentElement;
		currentElement = My402ListNext(linkedList, temp);
		// currentElement = temp->next;
		My402ListUnlink(linkedList, temp);
		// free(temp);
	}
	(linkedList->anchor).next = NULL;
	(linkedList->anchor).prev = NULL;
}

int  My402ListInsertAfter(My402List* linkedList, void* object, My402ListElem* element)
{
	if (element == NULL)
	{
		My402ListAppend(linkedList, object);
	}
	else
	{
		My402ListElem *newElement, *nextElement;
		newElement = (My402ListElem*)malloc(sizeof(My402ListElem));
		if (!newElement)
			{
				return FALSE;
			}
		// nextElement = My402ListNext(linkedList, element);
		nextElement = element->next;
		newElement->prev = element;
		newElement->next = nextElement;
		newElement->obj = object;
		element->next = newElement;
		nextElement->prev = newElement;
		linkedList->num_members++;
	}
	return TRUE;
}

int  My402ListInsertBefore(My402List* linkedList, void* object, My402ListElem* element)
{
	if (element == NULL)
	{
		My402ListPrepend(linkedList, object);
	}
	else
	{
		My402ListElem *newElement, *prevElement;
		newElement = (My402ListElem*)malloc(sizeof(My402ListElem));
		if (!newElement)
			{
				return FALSE;
			}
		// prevElement = My402ListPrev(linkedList, element);
		prevElement = element->prev;
		newElement->prev = prevElement;
		newElement->next = element;
		newElement->obj = object;
		element->prev = newElement;
		prevElement->next = newElement;
		linkedList->num_members++;
	}
	return TRUE;
}

int My402ListInit(My402List* linkedList)
{
	(linkedList->anchor).prev = NULL;
	(linkedList->anchor).next = NULL;
	(linkedList->anchor).obj = NULL;
	linkedList->num_members = 0;
	return TRUE;
}
