#include <stdlib.h>
#include <stdio.h>
#include "LL.h"

#ifdef DEBUG
#undef DEBUG
#endif

//TODO: Comment everything
//TODO: Test everything?

//////////////////////////////////////////////////////////////////////
// Creates a new list...
LinkedList *
LL_new ()
{
	LinkedList *list;

	list = malloc (sizeof (LinkedList));
	if (!list)
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

//////////////////////////////////////////////////////////////////////
// TODO: test this function
// Destroys the entire list
// Warning!  Does not free the list data! (only the list itself)
int
LL_Destroy (LinkedList * list)
{
	LL_node *node, *next;

	if (!list)
		return -1;

	node = &list->head;

	for (node = node->next; node && node->next; node = next) {
		// Avoid accessing "node" after it's freed..  :)
		next = node->next;
		if (LL_node_Destroy (node) < 0)
			return -1;
	}

	free (list);

	return 0;
}

//////////////////////////////////////////////////////////////////////
// TODO: test this function
// Warning!  This does not assert that the node data is free!
int
LL_node_Destroy (LL_node * node)
{
	if (!node)
		return -1;

	if (LL_node_Unlink (node) < 0)
		return -1;

	free (node);

	return 0;
}

//////////////////////////////////////////////////////////////////////
int
LL_node_Unlink (LL_node * node)
{
	LL_node *next, *prev;

	if (!node)
		return -1;

	next = node->next;
	prev = node->prev;

	if (next)
		next->prev = prev;
	if (prev)
		prev->next = next;

	node->next = NULL;
	node->prev = NULL;

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Frees the data in a list node, if not NULL...
int
LL_node_DestroyData (LL_node * node)
{
	if (!node)
		return -1;

	if (node->data)
		free (node->data);
	else
		return -1;
	return 0;
}

//////////////////////////////////////////////////////////////////////
// Returns to the beginning of the list...
int
LL_Rewind (LinkedList * list)
{
	if (!list)
		return -1;
/*
  printf("LL_Rewind:  list=%8x\n", list);
  printf("LL_Rewind:  list.head=%8x\n", &list->head);
  printf("LL_Rewind:  list.tail=%8x\n", &list->tail);
*/

	if (list->head.next != &list->tail)
		list->current = list->head.next;
	else
		list->current = &list->head;

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Goes to the end of the list...
int
LL_End (LinkedList * list)
{
	if (!list)
		return -1;

	if (list->tail.prev != &list->head)
		list->current = list->tail.prev;
	else
		list->current = &list->tail;

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Go to the next node
int
LL_Next (LinkedList * list)
{
	if (!list)
		return -1;
	if (!list->current)
		return -1;

	if (list->current->next != &list->tail) {
		list->current = list->current->next;
		return 0;
	} else {
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////
// Go to the previous node
int
LL_Prev (LinkedList * list)
{
	if (!list)
		return -1;
	if (!list->current)
		return -1;

	if (list->current->prev != &list->head) {
		list->current = list->current->prev;
		return 0;
	} else {
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////
// Data manipulation
void *
LL_Get (LinkedList * list)
{
	if (!list)
		return NULL;
	if (!list->current)
		return NULL;

	return list->current->data;
}

//////////////////////////////////////////////////////////////////////
int
LL_Put (LinkedList * list, void *data)
{
	if (!list)
		return -1;
	if (!list->current)
		return -1;

	list->current->data = data;

	return 0;
}

//////////////////////////////////////////////////////////////////////
LL_node *
LL_GetNode (LinkedList * list)
{
	if (!list)
		return NULL;

	return list->current;
}

//////////////////////////////////////////////////////////////////////
// Don't use this unless you know what you're doing.
int
LL_PutNode (LinkedList * list, LL_node * node)
{
	if (!list)
		return -1;
	if (!node)
		return -1;

	list->current = node;

	return 0;
}

//////////////////////////////////////////////////////////////////////
void *
LL_GetFirst (LinkedList * list)			  // gets data from first node
{
	if (!list)
		return NULL;

	if (0 > LL_Rewind (list))
		return NULL;

	return LL_Get (list);
}

//////////////////////////////////////////////////////////////////////
//
void *
LL_GetNext (LinkedList * list)			  //            ... next node
{
	if (!list)
		return NULL;

	if (0 > LL_Next (list))
		return NULL;

	return LL_Get (list);
}

//////////////////////////////////////////////////////////////////////
void *
LL_GetPrev (LinkedList * list)			  //            ... prev node
{
	if (!list)
		return NULL;

	if (0 > LL_Prev (list))
		return NULL;

	return LL_Get (list);
}

//////////////////////////////////////////////////////////////////////
void *
LL_GetLast (LinkedList * list)			  //            ... last node
{
	if (!list)
		return NULL;

	if (0 > LL_End (list))
		return NULL;

	return LL_Get (list);
}

//////////////////////////////////////////////////////////////////////
int
LL_AddNode (LinkedList * list, void *add)	// Adds node AFTER current one
{
	LL_node *node;

	if (!list)
		return -1;
	//if(!add) return -1;  // Nevermind..  NULL entries can be good...
	if (!list->current)
		return -1;

	//LL_dprint(list);

	node = malloc (sizeof (LL_node));
	if (!node)
		return -1;
	//printf("Allocated node\n");

/*   printf("Current: prev: %8x\tnode: %8x\tnext: %8x\n", */
/*   	 (int)list->current->prev, */
/*   	 (int)list->current, */
/*   	 (int)list->current->next); */
	if (list->current == &list->tail) {
		list->current = list->current->prev;
/*      printf("Was at end of list...\n"); */
/*      printf("Current: prev: %8x\tnode: %8x\tnext: %8x\n", */
/* 	    (int)list->current->prev, */
/* 	    (int)list->current, */
/* 	    (int)list->current->next); */
	}
//  printf("Setting node data\n");
	node->next = list->current->next;
	node->prev = list->current;
	node->data = add;
//  printf("...done\n");
/*      printf("NewNode: prev: %8x\tnode: %8x\tnext: %8x\n", */
/* 	    (int)node->prev, */
/* 	    (int)node, */
/* 	    (int)node->next); */

//  printf("Relinking...\n");
	if (node->next)
		node->next->prev = node;
//  printf("...\n");
	list->current->next = node;
//  printf("...done\n");

	list->current = node;

//  printf("Added node\n");
//  LL_dprint(list);

	return 0;
}

//////////////////////////////////////////////////////////////////////
int
LL_InsertNode (LinkedList * list, void *add)	// Adds node BEFORE current one
{
	LL_node *node;

	if (!list)
		return -1;
	if (!add)
		return -1;
	if (!list->current)
		return -1;

	node = malloc (sizeof (LL_node));
	if (!node)
		return -1;

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

////////////////////////////////////////////////////////////////////////
// Removes a node from the link
// ... and advances one node forward
void *
LL_DeleteNode (LinkedList * list)
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

/*
	printf ("LL_DeleteNode: Before...\n");
	LL_dprint (list);
*/

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

	free (list->current);

	list->current = next;

/*
	printf ("LL_DeleteNode: After...\n");
	LL_dprint (list);
*/

	return data;
}

//////////////////////////////////////////////////////////////////////
// Removes a specific node...
void *
LL_Remove (LinkedList * list, void *data)
{
	void *find;

	if (!list)
		return NULL;

	LL_Rewind (list);
	do {
		find = LL_Get (list);
		if (find == data)
			return LL_DeleteNode (list);
	} while (LL_Next (list) == 0);

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// Stack operations
int
LL_Push (LinkedList * list, void *add)  // Add node to end of list
{
	if (!list)
		return -1;
	if (!add)
		return -1;

//  printf("Going to end of list...\n");
	LL_End (list);

//  printf("Adding node...\n");
	return LL_AddNode (list, add);
}

//////////////////////////////////////////////////////////////////////
void *
LL_Pop (LinkedList * list)				  // Remove node from end of list
{
	if (!list)
		return NULL;

	if (0 > LL_End (list))
		return NULL;

	return LL_DeleteNode (list);
}

//////////////////////////////////////////////////////////////////////
void *
LL_Top (LinkedList * list)				  // Peek at end node
{
	return LL_GetLast (list);
}

//////////////////////////////////////////////////////////////////////
void *
LL_Shift (LinkedList * list)				  // Remove node from start of list
{
	if (!list)
		return NULL;

	if (0 > LL_Rewind (list))
		return NULL;

	return LL_DeleteNode (list);
}

//////////////////////////////////////////////////////////////////////
void *
LL_Look (LinkedList * list)				  // Peek at first node
{
	return LL_GetFirst (list);
}

//////////////////////////////////////////////////////////////////////
int
LL_Unshift (LinkedList * list, void *add)	// Add node to beginning of list
{
	if (!list)
		return -1;
	if (!add)
		return -1;

	LL_Rewind (list);

	return LL_InsertNode (list, add);
}

//////////////////////////////////////////////////////////////////////
int
LL_Roll (LinkedList * list)				  // Make last node first
{
	LL_node *node, *next;

	if (!list)
		return -1;
	//if(!list->current) return -1;

	if (0 > LL_End (list))
		return -1;

	// Avoid rolling an empty list, or unlinking the head/tail...
	if (list->current == &list->head)
		list->current = list->current->next;
	if (list->current == &list->tail)
		list->current = list->current->prev;
	// List is empty
	if (list->current == &list->head)
		return 0;
	// List has one item
	if (list->current->prev == &list->head)
		return 0;

	node = list->current;

	LL_node_Unlink (node);

	if (0 > LL_Rewind (list))
		return -1;

	next = list->head.next;

	list->head.next = node;
	next->prev = node;
	node->prev = &list->head;
	node->next = next;

	return 0;
}

//////////////////////////////////////////////////////////////////////
int
LL_UnRoll (LinkedList * list)			  // Roll the other way...
{
	LL_node *node, *prev;

	if (!list)
		return -1;
	//if(!list->current) return -1;

	if (0 > LL_Rewind (list))
		return -1;

	// Avoid rolling an empty list, or unlinking the head/tail...
	if (list->current == &list->tail)
		list->current = list->current->prev;
	if (list->current == &list->head)
		list->current = list->current->next;
	// List is empty
	if (list->current == &list->tail)
		return 0;
	// List has one item
	if (list->current->next == &list->tail)
		return 0;

	node = list->current;

	LL_node_Unlink (node);

	if (0 > LL_End (list))
		return -1;

	prev = list->tail.prev;

	list->tail.prev = node;
	prev->next = node;
	node->next = &list->tail;
	node->prev = prev;

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Add an item to the end of its "priority group"
// The list is assumed to be sorted already...
int
LL_PriorityEnqueue (LinkedList * list, void *add, int compare (void *, void *))
{
	void *data;
	int i;

	if (!list)
		return -1;
	if (!add)
		return -1;
	if (!compare)
		return -1;

	// From the end of the list, keep searching while we're "less than"
	// the given nodes...
	LL_End (list);
	do {
		data = LL_Get (list);
		if (data) {
			i = compare (add, data);
			if (i >= 0)				  // If we're in the right place, add it and exit
			{
				LL_AddNode (list, add);
				return 0;
			}
		}
	} while (LL_Prev (list) == 0);

	// If we're less than *everything*, put it at the beginning
	LL_Unshift (list, add);

	return 0;
}

//////////////////////////////////////////////////////////////////////
int
LL_SwapNodes (LL_node * one, LL_node * two)	// Switch two nodes positions...
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

//////////////////////////////////////////////////////////////////////
int
LL_nSwapNodes (int one, int two)	// Switch two nodes positions...
{
	return -1;
}

//////////////////////////////////////////////////////////////////////
int
LL_Length (LinkedList * list)			  // Returns # of nodes in entire list
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

//////////////////////////////////////////////////////////////////////
// Searching...
// Goes to the list item which matches "value", and returns the
// data found there.
//
// The "compare" function should return 0 for a "match"
//
// Note that this does *not* rewind the list first!  You should do
// it yourself if you want to start from the beginning!
void *
LL_Find (LinkedList * list, int compare (void *, void *), void *value)
{
	void *data;

	if (!list)
		return NULL;
	if (!compare)
		return NULL;
	if (!value)
		return NULL;

	do {
		data = LL_Get (list);
		if (0 == compare (data, value))
			return data;

	} while (LL_Next (list) == 0);

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// Array like...
// Goes to the nth list item, and returns the
// data found there.
//
void *
LL_GetByIndex (LinkedList * list, int index)
{
	LL_node *node;
	int num = 0;

	if (!list)
		return NULL;
	if (index<0)
		return NULL;

	for (node = list->head.next; node != &list->tail; node = node->next) {
		if (num == index)
			return node->data;
		num ++;
	}
	return NULL; // got past the end
}

//////////////////////////////////////////////////////////////////////
// Sorts the list, then rewinds it...
//
int
LL_Sort (LinkedList * list, int compare (void *, void *))
{
	int i, j;						  // Junk / loop variables
	int numnodes;					  // number of nodes in list
	LL_node *best, *last;		  // best match and last node in the list
	LL_node *current;

	if (!list)
		return -1;
	if (!compare)
		return -1;

	numnodes = LL_Length (list); // get the number of nodes...
	if (0 > LL_End (list))
		return -1;					  // Find the last node.
	last = LL_GetNode (list);

	if (numnodes < 2)
		return 0;

	for (i = numnodes - 1; i > 0; i--) {
		LL_Rewind (list);			  // get the first node again
		best = last;				  // reset our "best" node

		for (j = 0; j < i; j++) {
			current = LL_GetNode (list);
			// If we found a better match...
			if (compare (current->data, best->data) > 0) {
				best = current;	  // keep track of the "best" match
			}
			LL_Next (list);		  // Go to the next node.
		}

		LL_SwapNodes (last, best);	// Switch two nodes...
		if (best)
			last = best->prev;
		else
			return -1;

		//last = LL_FindPrev(best);         // And go backwards by one node.
	}

	//return LLFindFirst(current);        // return pointer to the first node.
	LL_Rewind (list);

	return 0;

}

void
LL_dprint (LinkedList * list)
{
	LL_node *current;

	current = &list->head;

	printf ("Head:  prev:\t0x%p\taddr:\t0x%p\tnext:\t0x%p\n", list->head.prev, &list->head, list->head.next);

	for (current = current->next; current != &list->tail; current = current->next) {
		printf ("node:  prev:\t0x%p\taddr:\t0x%p\tnext:\t0x%p\n", current->prev, current, current->next);
	}

	printf ("Tail:  prev:\t0x%p\taddr:\t0x%p\tnext:\t0x%p\n", list->tail.prev, &list->tail, list->tail.next);
}
