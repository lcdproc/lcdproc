#ifndef LL_H
#define LL_H



/***********************************************************************
  Linked Lists!  (Doubly-Linked Lists)
  *******************************************************************

  To create a list, do the following:

    LL *list;
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

    thingie = (my_data *)LL_DeleteNode(list);
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
    LL_Roll()

    LL_Shift()     // Other end of the stack (like in perl)
    LL_Unshift()
    LL_Look()
    LL_UnRoll()

    LL_Enqueue()   // Standard queue operations
    LL_Dequeue()

  There are also other goodies, like sorting and searching.

  *******************************************************************

  Array-like operations will come later, to allow numerical indexing:

    LL_nGet(list, 3);
    LL_nSwap(list, 6, 13);
    LL_nPut(list, -4, data);   // Puts item at 4th place from the end..

  More ideas for later:

    LL_MoveNode(list, amount);  // Slides a node to another spot in the list
    -- LL_MoveNode(list, -1); // moves a node back one toward the head

    ... um, more?

  *******************************************************************
  That's about it, for now...  Be sure to free the list when you're done!
***********************************************************************/


// See LL.c for more detailed descriptions of these functions.

typedef struct LL_node
{
  struct LL_node *next, *prev;
  void *data;
} LL_node;

typedef struct LL
{
  LL_node head, tail;
  LL_node *current;
} LL;



// Creates a new list...
LL * LL_new();
// Destroying lists...
int LL_Destroy(LL *list);
int LL_node_Destroy(LL_node *node);
int LL_node_Unlink(LL_node *node);
int LL_node_DestroyData(LL_node *node);

// Returns to the beginning of the list...
int LL_Rewind(LL *list);
// Goes to the end of the list...
int LL_End(LL *list);
// Go to the next node
int LL_Next(LL *list);
// Go to the previous node
int LL_Prev(LL *list);

// Data manipulation
void * LL_Get(LL *list);
int LL_Put(LL *list, void *data);
// Don't use these next two unless you really know what you're doing.
LL_node * LL_GetNode(LL *list);
int LL_PutNode(LL *list, LL_node *node);

void * LL_GetFirst(LL *list);	// gets data from first node
void * LL_GetNext (LL *list);	//            ... next node
void * LL_GetPrev (LL *list);	//            ... prev node
void * LL_GetLast (LL *list);	//            ... last node

int LL_AddNode(LL *list, void * add);	// Adds node AFTER current one
int LL_InsertNode(LL *list, void * add);// Adds node BEFORE current one
// Removes a node from the link; returns the data from the node
void * LL_DeleteNode(LL *list);
// Removes a specific node...
void * LL_Remove(LL *list, void * data);

// Stack operations
int LL_Push(LL *list, void *add);    // Add node to end of list
void * LL_Pop(LL *list);	     // Remove node from end of list
void * LL_Top(LL *list);	     // Peek at end node
void * LL_Shift(LL *list);	     // Remove node from start of list
void * LL_Look(LL *list);            // Peek at first node
int LL_Unshift(LL *list, void *add); // Add node to beginning of list

int LL_Roll(LL *list);  // Make first node last
int LL_UnRoll(LL *list);// Roll the other way...

// Queue operations...
//int LL_Enqueue(LL *list, void *add);
//void * LL_Dequeue(LL *list);
//////////////////////////////////////////////////////////////////////
// Queue operations...
#define LL_Enqueue(list,add) LL_Push(list,add)

#define LL_Dequeue(list) LL_Shift(list)

int LL_PriorityEnqueue(LL *list, void *add, int compare(void *, void *));


int LL_SwapNodes(LL_node *one, LL_node *two); // Switch two nodes positions...
int LL_nSwapNodes(int one, int two);   // Switch two nodes positions...

int LL_Length(LL *list);	// Returns # of nodes in entire list

// Searching...
void * LL_Find(LL *list, int compare(void *, void *), void *value);

// Sorts the list...
int LL_Sort(LL *list, int compare(void *, void *));


// Debugging...
void LL_dprint(LL *list);

#endif
