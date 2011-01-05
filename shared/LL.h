/** \file shared/LL.h
 * Define routines to deal with doubly linked lists
 */

/* This file is part of LCDproc.
 *
 * This file is released under the GNU General Public License.
 * Refer to the COPYING file distributed with this package.
 *
 * Copyright(c) 1994, Selene Scriven
 *          (c) 1999, William Ferrell
 *
 */

#ifndef LL_H
#define LL_H

/***********************************************************************
  Linked Lists!  (Doubly-Linked Lists)
  *******************************************************************

  To create a list, do the following:

    LinkedList *list;
    list = LL_new();
    if(!list) handle_an_error();

  The list can hold any type of data.  You will need to typecast your
  datatype to a "void *", though.  So, to add something to the list,
  the following would be a good way to start:

    typedef struct my_data {
      char string[16];
      int number;
    } my_data;

    my_data *thingie;

    for(something to something else)
    {
      thingie = malloc(sizeof(my_data));
      LL_AddNode(list, (void *)thingie);  // typecast it to a "void *"
    }

    For errors, the general convention is that "0" means success, and
    a negative number means failure.  Check LL.c to be sure, though.

  *******************************************************************

  To change the data, try this:

    thingie = (my_data *)LL_Get(list);  // typecast it back to "my_data"
    thingie->number = another_number;

  You don't need to "Put" the data back, but it doesn't hurt anything.

    LL_Put(list, (void *)thingie);

  However, if you want to point the node's data somewhere else, you'll
  need to get the current data first, keep track of it, then set the data
  to a new location:

    my_data * old_thingie, new_thingie;

    old_thingie = (my_data *)LL_Get(list);
    LL_Put(list, (void *)new_thingie);

    // Now, do something with old_thingie.  (maybe, free it?)

  Or, you could just delete the node entirely and then add a new one:

    my_data * thingie;

    thingie = (my_data *)LL_DeleteNode(list, NEXT);
    free(thingie);

    thingie->number = 666;

    LL_InsertNode(list, (void *)thingie);

  *******************************************************************

  To operate on each list item, try this:

    LL_Rewind(list);
    do {
      my_data = (my_data *)LL_Get(list);
      ... do something to it ...
    } while(LL_Next(list) == 0);

  *******************************************************************

  You can also treat the list like a stack, or a queue.  Just use the
  following functions:

    LL_Push()      // Regular stack stuff: add, remove, peek, rotate
    LL_Pop()
    LL_Top()

    LL_Shift()     // Other end of the stack (like in perl)
    LL_Unshift()
    LL_Look()

    LL_Enqueue()   // Standard queue operations
    LL_Dequeue()

  There are also other goodies, like sorting and searching.

  *******************************************************************
  That's about it, for now...  Be sure to free the list when you're done!
***********************************************************************/

// See LL.c for more detailed descriptions of these functions.


/** Symbolic values for directions */
typedef enum _direction {
	HEAD     = -2,
	PREV     = -1,
	CURRENT	 = 0,
	NEXT     = +1,
	TAIL     = +2
} Direction;


/** Structure for a node in a linked list */
typedef struct LL_node {
	struct LL_node *prev;	/**< Pointer to previous node */
	struct LL_node *next;	/**< Pointer to next node */
	void *data;		/**< Payload */
} LL_node;


/** Structire for a linked list */
typedef struct LinkedList {
	LL_node head;		/**< List's head anchor */
	LL_node tail;		/**< List's tail anchor */
	LL_node *current;	/**< Pointer to current node */
} LinkedList;


// Creates a new list...
LinkedList *LL_new(void);
// Destroying lists...
int LL_Destroy(LinkedList *list);

// move "current" node
LL_node *LL_GoTo(LinkedList *list, Direction whereto);

// Returns to the beginning of the list...
int LL_Rewind(LinkedList *list);
// Goes to the end of the list...
int LL_End(LinkedList *list);
// Go to the next node
int LL_Next(LinkedList *list);
// Go to the previous node
int LL_Prev(LinkedList *list);

// Data manipulation
void *LL_Get(LinkedList *list);
int LL_Put(LinkedList *list, void *data);
// Don't use these next two unless you really know what you're doing.
LL_node *LL_GetNode(LinkedList *list);
int LL_PutNode(LinkedList *list, LL_node *node);

void *LL_GetFirst(LinkedList *list);  // gets data from first node
void *LL_GetNext(LinkedList *list);	  //            ... next node
void *LL_GetPrev(LinkedList *list);	  //            ... prev node
void *LL_GetLast(LinkedList *list);	  //            ... last node

int LL_AddNode(LinkedList *list, void *add);	// Adds node AFTER current one
int LL_InsertNode(LinkedList *list, void *add);	// Adds node BEFORE current one
// Removes a node from the link; returns the data from the node
void *LL_DeleteNode(LinkedList *list, Direction whereto);
// Removes a specific node...
void *LL_Remove(LinkedList *list, void *data, Direction whereto);

// Stack operations
int LL_Push(LinkedList *list, void *add);	// Add node to end of list
void *LL_Pop(LinkedList *list);		  // Remove node from end of list
void *LL_Top(LinkedList *list);		  // Peek at end node
void *LL_Shift(LinkedList *list);	  // Remove node from start of list
void *LL_Look(LinkedList *list);		  // Peek at first node
int LL_Unshift(LinkedList *list, void *add);	// Add node to beginning of list

// Queue operations...
#define LL_Enqueue(list,add)	LL_Push(list,add)
#define LL_Dequeue(list)	LL_Shift(list)

int LL_PriorityEnqueue(LinkedList * list, void *add, int (*compare)(void *, void *));

int LL_SwapNodes(LL_node *one, LL_node *two);	// Switch two nodes positions...

int LL_Length(LinkedList *list);		  // Returns # of nodes in entire list

// Searching...
void *LL_Find(LinkedList *list, int (*compare)(void *, void *), void *value);

void LL_ForAll(LinkedList *list, void *(*action)(void *, void *), void *value);

// Array operation...
void *LL_GetByIndex(LinkedList *list, int index);  // gets the nth node, 0 being the first

// Sorts the list...
int LL_Sort(LinkedList *list, int (*compare)(void *, void *));

// Debugging...
void LL_dprint(LinkedList *list);

#endif
