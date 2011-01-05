/** \file shared/LL.c
 * Define routines to deal with doubly linked lists
 */

/* This file is part of LCDproc.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright(c) 1994, Selene Scriven
 *          (c) 1999, William Ferrell
 *          (c) 2000, Guillaume Filion
 *          (c) 2001, Joris Robijn
 *          (c) 2008, Peter Marschall
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "LL.h"

#ifdef DEBUG
#undef DEBUG
#endif

//TODO: Test everything?


/** Create new linked list.
 * \return  Pointer to freshly created list object; \c NULL on error.
 */
LinkedList *
LL_new(void)
{
	LinkedList *list;

	list = malloc(sizeof(LinkedList));
	if (list == NULL)
		return NULL;

	list->head.data = NULL;
	list->head.prev = NULL;
	list->head.next = &list->tail;
	list->tail.data = NULL;
	list->tail.prev = &list->head;
	list->tail.next = NULL;
	list->current = &list->head;

	return list;
}


/** Destroy the entire list.
 *
 * \note
 * This does not free the data, only the list itself.
 *
 * \param list   List object to be destroyed.
 * \retval <0    error
 * \retval  0	 success
 */
int
LL_Destroy(LinkedList *list)
{
	LL_node *prev, *next;
	LL_node *node;

	if (!list)
		return -1;

	node = &list->head;

	for (node = node->next; node && node->next; node = next) {
		// Avoid accessing "node" after it's freed..  :)
		next = node->next;
		prev = node->prev;

		if (next != NULL)
			next->prev = prev;
		if (prev != NULL)
			prev->next = next;

		node->next = NULL;
		node->prev = NULL;

		free(node);
	}

	free(list);

	return 0;
}


/** Move to another entry in the list.
 * Set list's \c current pointer to the node denoted to by \c whereto.
 * \param list     List object.
 * \param whereto  Direction where to set the list's \c current pointer
 * \return         New value of list's \c current pointer;
 *                 \c NULL on error or when moving beyond ends.
 */
LL_node *
LL_GoTo(LinkedList *list, Direction whereto)
{
	if (!list)
		return NULL;

	switch (whereto) {
		case HEAD:	list->current = (list->head.next != &list->tail)
						? list->head.next
						: NULL;
				break;
		case PREV:	if (list->current->prev == &list->head)
					return NULL;
				list->current = list->current->prev;
		case CURRENT:	break;
		case NEXT:	if (list->current->next == &list->tail)
					return NULL;
				list->current = list->current->next;
				break;
		case TAIL:	list->current = (list->tail.prev != &list->head)
						? list->tail.prev
						: NULL;
				break;
	}

	return list->current;
}


/** Return to the beginning of the list.
 * Set list's \c current pointer to the first node in the list.
 * \param list   List object.
 * \retval <0    error: no list given
 * \retval  0	 success
 */
int
LL_Rewind(LinkedList *list)
{
	if (!list)
		return -1;

	list->current = (list->head.next != &list->tail)
			? list->head.next
			: &list->head;

	return 0;
}


/** Jump to the end of the list.
 * Set list's \c current pointer to the last node in the list.
 * \param list   List object.
 * \retval <0    error: no list given
 * \retval  0	 success
 */
int
LL_End(LinkedList *list)
{
	if (!list)
		return -1;

	list->current = (list->tail.prev != &list->head)
			? list->tail.prev
			: &list->tail;

	return 0;
}


/** Go to the next node of the list.
 * Advance list's \c current pointer to the next node in the list.
 * \param list   List object.
 * \retval <0    error: no list given or no next node
 * \retval  0	 success
 */
int
LL_Next(LinkedList *list)
{
	if (!list)
		return -1;
	if (!list->current)
		return -1;

	if (list->current->next == &list->tail)
		return -1;

	list->current = list->current->next;
	return 0;
}


/** Go to the previous node of the list.
 * Set list's \c current pointer to the previous node in the list.
 * \param list   List object.
 * \retval <0    error: no list given or no previous node
 * \retval  0	 success
 */
int
LL_Prev(LinkedList *list)
{
	if (!list)
		return -1;
	if (!list->current)
		return -1;

	if (list->current->prev == &list->head)
		return -1;

	list->current = list->current->prev;
	return 0;
}


/** Access current node's data.
 * Return pointer to list's \c current node's data.
 * \param list   List object.
 * \return       Pointer to \c current node's payload data;
 *               \c NULL may be empty payload or an error.
 */
void *
LL_Get(LinkedList *list)
{
	if (!list)
		return NULL;
	if (!list->current)
		return NULL;

	return list->current->data;
}


/** Set/change current node's data.
 * \param list   List object.
 * \param data   Pointer to data to be set.
 * \retval <0    error: no list given, or no current node
 * \retval  0	 success
 */
int
LL_Put(LinkedList *list, void *data)
{
	if (!list)
		return -1;
	if (!list->current)
		return -1;

	list->current->data = data;

	return 0;
}


/** Get current node in list.
 * \param list   List object.
 * \return       Pointer to current node.
 */
LL_node *
LL_GetNode(LinkedList *list)
{
	if (!list)
		return NULL;

	return list->current;
}


/** Set list's \c current pointer to a specific node.
 *
 * \warning
 * Don't use this unless you know what you're doing.
 *
 * \param list   List object.
 * \param node   Node to become new \c current.
 * \retval <0    error
 * \retval  0    success
 */
int
LL_PutNode(LinkedList *list, LL_node *node)
{
	if (!list)
		return -1;
	if (!node)
		return -1;

	list->current = node;

	return 0;
}


/** Access list's first node's data.
 * Set list's \c current pointer to the first node and return its data.
 * \param list   List object.
 * \return       Pointer to first node's data; \c NULL on error.
 */
void *
LL_GetFirst(LinkedList *list)
{
	if (!list)
		return NULL;

	if (0 > LL_Rewind(list))
		return NULL;

	return LL_Get(list);
}


/** Access next node's data.
 * Advance list's \c current pointer to the next node and return its data.
 * \param list   List object.
 * \return       Pointer to next node's data; \c NULL on error.
 */
void *
LL_GetNext(LinkedList *list)
{
	if (!list)
		return NULL;

	if (0 > LL_Next(list))
		return NULL;

	return LL_Get(list);
}


/** Access previous node's data.
 * Set list's \c current pointer to the previous node, and return its data.
 * \param list   List object.
 * \return       Pointer to previous node's data; \c NULL on error.
 */
void *
LL_GetPrev(LinkedList *list)
{
	if (!list)
		return NULL;

	if (0 > LL_Prev(list))
		return NULL;

	return LL_Get(list);
}


/** Access list's last node's data.
 * Set list's \c current pointer to the last node and return its data.
 * \param list   List object.
 * \return       Pointer to last node's data; \c NULL on error.
 */
void *
LL_GetLast(LinkedList *list)
{
	if (!list)
		return NULL;

	if (0 > LL_End(list))
		return NULL;

	return LL_Get(list);
}


/** Add/append a new node after current one in the list.
 * Update the list's \c current pointer to point to the freshly created node.
 * \param list   List object.
 * \param add    Pointer to new node's data.
 * \retval <0    error
 * \retval  0	 success
 */
int
LL_AddNode(LinkedList *list, void *add)
{
	LL_node *node;

	if (!list)
		return -1;
	if (!list->current)
		return -1;

	node = malloc(sizeof(LL_node));
	if (node == NULL)
		return -1;

	// we're behind the list's end, go to previous node
	if (list->current == &list->tail)
		list->current = list->current->prev;

	// Set node data
	node->next = list->current->next;
	node->prev = list->current;
	node->data = add;

	// Re-link
	if (node->next)
		node->next->prev = node;
	list->current->next = node;

	list->current = node;

	return 0;
}


/** Add/insert a new node before current one in the list.
 * Update the list's \c current pointer to point to the freshly created node.
 * \param list   List object.
 * \param add    Pointer to new node's data.
 * \retval <0    error
 * \retval  0	 success
 */
int
LL_InsertNode(LinkedList *list, void *add)
{
	LL_node *node;

	if (!list)
		return -1;
	if (!add)
		return -1;
	if (!list->current)
		return -1;

	node = malloc(sizeof(LL_node));
	if (node == NULL)
		return -1;

	// we're before the list's start, go to next node
	if (list->current == &list->head)
		list->current = list->current->next;

	node->next = list->current;
	node->prev = list->current->prev;
	node->data = add;

	if (list->current->prev)
		list->current->prev->next = node;
	list->current->prev = node;

	list->current = node;

	return 0;
}


/** Remove current node from the list.
 * Set the list's \c current pointer to the one denoted by \c whereto.
 * \param list     List object.
 * \param whereto  Direction where to set the list's \c current pointer
 * \return         Pointer to data of deleted node; \c NULL on error.
 */
void *
LL_DeleteNode(LinkedList *list, Direction whereto)
{
	LL_node *next, *prev;
	void *data;

	if (!list)
		return NULL;
	if (!list->current)
		return NULL;
	if (list->current == &list->head)
		return NULL;
	if (list->current == &list->tail)
		return NULL;

	next = list->current->next;
	prev = list->current->prev;
	data = list->current->data;

	if (prev)
		prev->next = next;

	if (next)
		next->prev = prev;

	list->current->prev = NULL;
	list->current->next = NULL;
	// This should not free things; the user should do it explicitly.
	//if(list->current->data) free(list->current->data);
	list->current->data = NULL;

	free(list->current);

	switch (whereto) {
		case HEAD:	list->current = list->head.next;
				break;
		case TAIL:	list->current = list->tail.prev;
				break;
		case PREV:	list->current = prev;
				break;
		default:
		case NEXT:	list->current = next;
	}

	return data;
}


/** Remove a specific node from the list.
 * Find a node by a pointer to its data and remove it.
 * Set the list's \c current pointer to the one denoted by \c whereto.
 * \param list   List object.
 * \param data   Pointer to data of node to delete.
 * \param whereto  Direction where to set the list's \c current pointer
 * \return       Pointer to data of deleted node; \c NULL on error.
 */
void *
LL_Remove(LinkedList *list, void *data, Direction whereto)
{
	if (!list)
		return NULL;

	LL_Rewind(list);
	do {
		void *find = LL_Get(list);

		if (find == data)
			return LL_DeleteNode(list, whereto);
	} while (LL_Next(list) == 0);

	return NULL;
}


/** Add/append a new node after the last one in the list.
 * Jump to the last node in the list, append a new node
 * and make this new one the list's \c current one.
 * \param list   List object.
 * \param add    Pointer to new node's data.
 * \retval <0    error
 * \retval  0	 success
 */
int
LL_Push(LinkedList *list, void *add)
{
	if (!list)
		return -1;
	if (!add)
		return -1;

	LL_End(list);

	return LL_AddNode(list, add);
}


/** Remove the last node from the list, and return its data.
 * Jump to the last node in the list, remove it from the list
 * and return its data.
 * \param list   List object.
 * \return       Pointer to data of deleted node; \c NULL on error.
 */
void *
LL_Pop(LinkedList *list)
{
	if (!list)
		return NULL;

	if (0 > LL_End(list))
		return NULL;

	return LL_DeleteNode(list, PREV);
}


/** Access list's last node's data.
 * Set list's \c current pointer to the last node and return its data.
 * \param list   List object.
 * \return       Pointer to last node's data; \c NULL on error.
 */
void *
LL_Top(LinkedList *list)
{
	return LL_GetLast(list);
}


/** Remove the first node from the list, and return its data.
 * Jump to the first node in the list, remove it from the list and return its data.
 * \param list   List object.
 * \return       Pointer to data of deleted node; \c NULL on error.
 */
void *
LL_Shift(LinkedList *list)
{
	if (!list)
		return NULL;

	if (0 > LL_Rewind(list))
		return NULL;

	return LL_DeleteNode(list, NEXT);
}


/** Access list's first node's data.
 * Set list's \c current pointer to the first node and return its data.
 * \param list   List object.
 * \return       Pointer to first node's data; \c NULL on error.
 */
void *
LL_Look(LinkedList *list)
{
	return LL_GetFirst(list);
}


/** Add/insert a new node before the first one in the list.
 * Jump to the first node in the list and insert a new node before that one.
 * \param list   List object.
 * \param add    Pointer to new node's data.
 * \retval <0    error
 * \retval  0	 success
 */
int
LL_Unshift(LinkedList *list, void *add)
{
	if (!list)
		return -1;
	if (!add)
		return -1;

	LL_Rewind(list);

	return LL_InsertNode(list, add);
}


/** Add an item to the end of its "priority group"
 * The list is assumed to be sorted already.
 * \param list     List object.
 * \param add      Pointer to new node's data.
 * \param compare  Pointer to a comparison function.
 * \retval <0      error
 * \retval  0      success
 */
int
LL_PriorityEnqueue(LinkedList *list, void *add, int (*compare)(void *, void *))
{
	if (!list)
		return -1;
	if (!add)
		return -1;
	if (!compare)
		return -1;

	// From the end of the list, keep searching while we're "less than"
	// the given nodes...
	LL_End(list);
	do {
		void *data = LL_Get(list);

		if (data) {
			int i = compare(add, data);

			if (i >= 0) {	// If we're in the right place, add it and exit
				LL_AddNode(list, add);
				return 0;
			}
		}
	} while (LL_Prev(list) == 0);

	// If we're less than *everything*, put it at the beginning
	LL_Unshift(list, add);

	return 0;
}


/** Switch two nodes positions.
 * \param one   First list object.
 * \param two   Second list object.
 * \return      -1 on error, 0 on success
 */
int
LL_SwapNodes(LL_node *one, LL_node *two)
{
	LL_node *firstprev, *firstnext;
	LL_node *secondprev, *secondnext;

	if (!one || !two)
		return -1;
	if (one == two)
		return 0;					  // Do nothing

	firstprev = one->prev;		  // Look up the nodes neighbors...
	firstnext = one->next;
	secondprev = two->prev;
	secondnext = two->next;

	if (firstprev != NULL)
		firstprev->next = two;	  // Swap the neighboring
	if (firstnext != NULL)
		firstnext->prev = two;	  // nodes pointers...
	if (secondprev != NULL)
		secondprev->next = one;
	if (secondprev != NULL)
		secondnext->prev = one;

	one->next = secondnext;		  // Swap the nodes pointers
	one->prev = secondprev;
	two->next = firstnext;
	two->prev = firstprev;

	if (firstnext == two)
		one->prev = two;			  // Fix things in case
	if (firstprev == two)
		one->next = two;			  // they were next to
	if (secondprev == one)
		two->next = one;			  // each other...
	if (secondnext == one)
		two->prev = one;

	return 0;
}


/** Calculate the length of a list.
 * \param list   List object.
 * \return       Number of nodes in the list; \c -1 on error.
 */
int
LL_Length(LinkedList *list)
{
	LL_node *node;
	int num = 0;

	if (!list)
		return -1;

	node = &list->head;

	for (num = -1; node != &list->tail; num++)
		node = node->next;

	return num;
}


/** Find a node by giving a comparison function and a value.
 * Go to to the list node whose data matches the given value
 * and return the data.
 *
 * \note
 * This does \em not rewind the list first!
 * Do it yourself if you want to start from the beginning!
 *
 * \param list     List object.
 * \param compare  Pointer to a comparison function, that takes to void pointers
 *                 as arguments and returns an int. If must return \c 0 exactly
 *                 when the node's data matches \c value.
 * \param value    Pointer to the value used for matching.
 * \return         The found node's data pointer; \c NULL otherwise
 */
void *
LL_Find(LinkedList *list, int (*compare)(void *, void *), void *value)
{
	if (!list)
		return NULL;
	if (!compare)
		return NULL;
	if (!value)
		return NULL;

	do {
		void *data = LL_Get(list);

		if (0 == compare(data, value))
			return data;
	} while (LL_Next(list) == 0);

	return NULL;
}


/** Perform an action for the all list elements.
 * Execute a function on the data of each node in the list.
 * Depending on the result of the function, new nodes may get added,
 * nodes may get deleted or simply changed by the function itself.
 *
 * The \c action() function is in turn called with each node's data
 * pointer as the first argument and \c value as its second argument.
 * If it returns \c NULL, the node will be deleted from the list,
 * otherwise, if the pointer returned from the function differs from
 * data, a new node gets added after the current node.
 * If the result equals the payload data, no addition or deletio happens.
 *
 * \note
 * Removing the client payload in case of deletion, or creation of
 * the payload for the new node in case of addition is up to the
 * \c action() function.
 *
 * \note
 * \c value can be used to report errors, pass additional information, ...
 *
 * \param list     List object.
 * \param action   Pointer to the action function that takes two void pointers
 *                 as arguments and returns a void pointer.
 * \param value    Pointer to data that is used as second argument to
 *                 \c action().
 */
void
LL_ForAll(LinkedList *list, void *(*action)(void *, void *), void *value)
{
	if (!list)
		return;
	if (!action)
		return;

	LL_Rewind(list);
	if (list->current != NULL) {
		do {
			void *data = LL_Get(list);
			void *result = action(data, value);

			if (result != data) {
				if (result != NULL)
					LL_AddNode(list, result);
				else
					LL_DeleteNode(list, PREV);
			}
		} while (LL_Next(list) == 0);
	}
}


/** Go to the n-th node in the list and return its data.
 * Go to to the list node with the given \c index
 * and return the data.
 * \param list     List object.
 * \param index    Index of the node whose data we want.
 * \return         The found node's data pointer; \c NULL otherwise
 */
void *
LL_GetByIndex(LinkedList *list, int index)
{
	LL_node *node;
	int num = 0;

	if (!list)
		return NULL;
	if (index < 0)
		return NULL;

	for (node = list->head.next; node != &list->tail; node = node->next) {
		if (num == index)
			return node->data;
		num++;
	}

	return NULL; // got past the end
}


/** Sort list by its contents.
 * The list gets sorted using a comparison function for the data of its nodes.
 * After the sorting, the list's current pointer is set to the first node.
 * \param list     List object.
 * \param compare  Pointer to a comparison function, that takes to void pointers
 *                 as arguments and returns an int > \c 0 when the first argument
 *                 is considered greater than the second.
 * \retval <0      error
 * \retval  0      success.
 */
int
LL_Sort(LinkedList *list, int (*compare)(void *, void *))
{
	int i, j;			  // Junk / loop variables
	int numnodes;			  // number of nodes in list
	LL_node *best, *last;		  // best match and last node in the list
	LL_node *current;

	if (!list)
		return -1;
	if (!compare)
		return -1;

	numnodes = LL_Length(list); // get the number of nodes...
	if (0 > LL_End(list))
		return -1;				  // Find the last node.
	last = LL_GetNode(list);

	if (numnodes < 2)
		return 0;

	for (i = numnodes - 1; i > 0; i--) {
		LL_Rewind(list);			  // get the first node again
		best = last;				  // reset our "best" node

		for (j = 0; j < i; j++) {
			current = LL_GetNode(list);
			// If we found a better match...
			if (compare(current->data, best->data) > 0) {
				best = current;	  // keep track of the "best" match
			}
			LL_Next(list);		  // Go to the next node.
		}

		LL_SwapNodes(last, best);	// Switch two nodes...
		if (best)
			last = best->prev;
		else
			return -1;
	}

	LL_Rewind(list);

	return 0;
}


void
LL_dprint(LinkedList *list)
{
	LL_node *current;

	current = &list->head;

	printf("Head:  prev:\t0x%p\taddr:\t0x%p\tnext:\t0x%p\n", list->head.prev, &list->head, list->head.next);

	for (current = current->next; current != &list->tail; current = current->next) {
		printf("node:  prev:\t0x%p\taddr:\t0x%p\tnext:\t0x%p\n", current->prev, current, current->next);
	}

	printf("Tail:  prev:\t0x%p\taddr:\t0x%p\tnext:\t0x%p\n", list->tail.prev, &list->tail, list->tail.next);
}
