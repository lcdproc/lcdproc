#include <stdio.h>	// for NULL definition
#include "llistd.h"	// function prototypes

/*****************************************************************************
* AUTHOR: Scott Scriven, SSN: 523-21-9640, CLASS: CS151.003, DATE: 96-11-21
******************************************************************************
* Doubly Linked Lists!  (some of my really old code, commented for this class)
*  This code will handle "generic" Doubly-Linked Lists.  These lists can be
*  any type as long as they satisfy a few requirements:
*	Your structure must start with two integer pointers:
*		struct mystruct {
*		  int *next, *prev;
*		  ...
*		};
*	That's all, pretty much.
*
*  Most of the functions take integer pointers.  This means you will have to
*  call them in one of the following ways:
*	function((int *)&mystruct);
*	function((int *)mystruct);
*	function(&mystruct.next);
*
******************************************************************************
*  If I'm not mistaken, this code should work regardless of your machine's
*  integer size (16/32/64/etc... bit)
******************************************************************************
* Note that we have to do NULL checking to make sure we don't try to write
* to protected areas of memory if we reach the beginning/end of a list.
*
* We also do some circular-link checking to avoid infinite loops.
*	These loops will work fine:
* 		a <-> b <-> c <-> a <-> b...
*		a <-> b <-> c -> c -> c...
*	This loop might make this code puke:
*		a <-> b <-> c <-> d -> c    d <- e <-> f <-> g
*****************************************************************************/


/*****************************************************************************
* Function:  LLMakeFirstNode()
******************************************************************************
* Description:
*  Makes a node stand alone; creates the first node of a list.  You must do
*  the memory/node allocation manually.  It returns the parameter you send it.
******************************************************************************
* Function Calls:
*  None.
*****************************************************************************/
int * LLMakeFirstNode(int * first)	// Make a standalone node...
{
  (int *)first[0] = NULL;
  (int *)first[1] = NULL;
  return first;
}


/*****************************************************************************
* Function:  LLFindFirst()
******************************************************************************
* Description:
*  Given a node of a list, it returns the first node of the list (assuming
*  that the first node either points to NULL or itself).  If list is circular,
*  it returns *next* node in the list.
******************************************************************************
* Function Calls:
*  LLFindPrev		See below...
*****************************************************************************/
int * LLFindFirst(int * current)	// returns address of First node
{
  int *prev=current;

  // If prev[1] (pointer to previous node) is NULL, we hit the beginning.
  // If prev[1] is the same as current, we've got a circular linked list...
  while((int *)prev[1] != NULL  && (int *)prev[1] != current)
  {
    prev = LLFindPrev(prev);
  }
  
  return prev;
}

/*****************************************************************************
* Function:  LLFindNext()
******************************************************************************
* Description:
*  Returns the address of the next node in the list, or NULL if we're at the
*  end of the list.
******************************************************************************
* Function Calls:
*  None.
*****************************************************************************/
int * LLFindNext(int * current)		// returns address of next node
{
  // Return the address of the next node
  if((int *)current[0] != current)
    return (int *)current[0];
  else
    return NULL;
}

/*****************************************************************************
* Function:  LLFindPrev()
******************************************************************************
* Description:
*  Returns the address of the previous node in the list, or NULL if we're
*  at the beginning of the list.
******************************************************************************
* Function Calls:
*  None.
*****************************************************************************/
int * LLFindPrev(int * current)		// returns address of prev node
{
  // Return the address of the previous node
  if((int *)current[1] != current)   
    return (int *)current[1];
  else
    return NULL;
}

/*****************************************************************************
* Function:  LLFindLast()
******************************************************************************
* Description:
*  Given any node, returns the address of the last node of that list.  This
*  works just like LLFindFirst(), but in reverse.
******************************************************************************
* Function Calls:
*  LLFindNext		See above...
*****************************************************************************/
int * LLFindLast(int * current)		// returns address of last node
{
  int *next = current;
  
  // If next[0] (pointer to next node) is NULL, we hit the end.
  // If next[0] is the same as current, we've got a circular linked list...
  while((int *)next[0] != NULL  &&  (int *)next[0] != current)
  {
    next = LLFindNext(next);
  }
  
  return next;
}


/*****************************************************************************
* Function:  LLAddNode()
******************************************************************************
* Description:
*  Inserts a node in the list *after* the "current" node.  It updates the
*  list neighbors accordingly.
******************************************************************************
* Function Calls:
*  LLFindNext		See above...
*****************************************************************************/
int * LLAddNode(int * current, int * add)	// Adds node AFTER current one
{
  int *next;
  
  if(current == add) return current;	// We can't add a node to itself...
  
  next = LLFindNext(current);	// Get the next node address

  (int *)current[0] = add;	// Point the current node to the new node
  (int *)add[0] = next;		// Point the new node to next node
  (int *)add[1] = current;	// Make new node point back to the current node

  if(next != NULL)
    (int *)next[1] = add;	// Point next node back to the new node
  
  return add;		// Return the address of the new node
}

/*****************************************************************************
* Function:  LLInsertNode()
******************************************************************************
* Description:
*  Inserts a node in the list *before* the "current" node.  It updates the
*  list neighbors accordingly.
******************************************************************************
* Function Calls:
*  LLFindPrev		See above...
*****************************************************************************/
int * LLInsertNode(int * current, int * add)	// Adds node BEFORE current one
{
  int *prev;
  
  if(current == add) return current;	// We can't add a node to itself...

  prev = LLFindPrev(current);	// Get the previous node's address
  
  if(prev != NULL)
    (int *)prev[0] = add;	// Previous node points to new node
  (int *)add[0] = current;	// New node points to current node
  (int *)add[1] = prev;		// New node points back to previous node
  (int *)current[1] = add;	// Current node points back to new node
  
  return add;		// Return the address of the new node
}

/*****************************************************************************
* Function:  LLCutNode()
******************************************************************************
* Description:
*  Removes a node from the list, and joins its neighbors.  Returns the
*  address of the cut node.  You must deallocate the node manually, if
*  you want to free its memory.
******************************************************************************
* Function Calls:
*  LLFindPrev		See Above...
*  LLFindNext		See Above...
*****************************************************************************/
int * LLCutNode(int * cut)			// Removes a node from the link
{
  int * prev, * next;
  
  prev = LLFindPrev(cut);	// Find surrounding nodes...
  next = LLFindNext(cut);

  (int *)cut[0] = NULL;		// Make the removed node point nowhere
  (int *)cut[1] = NULL;
  
  if(prev != NULL)		// Join the surrounding nodes...  
    (int *)prev[0] = next;
  if(next != NULL)
    (int *)next[1] = prev;
  
  return cut;		// Return the address of the node that has been cut...
}


/*****************************************************************************
* Function:  LLPush()
******************************************************************************
* Description:
*  Inserts a node at the end of the whole list.  "Current" can be any node
*  in the list, and "add" is the node you want to add.
******************************************************************************
* Function Calls:
*  LLFindLast()		See above...
*  LLAddNode()		See above...
*****************************************************************************/
int * LLPush(int * current, int * add)	   // Add node to end of list
{
  int *last;
  
  last = LLFindLast(current);
  
  return LLAddNode(last, add);
}

/*****************************************************************************
* Function:  LLPop()
******************************************************************************
* Description:
*  Pops (removes) a node off the end of the list.  Returns the address of
*  the node we chopped off.  "current" can be any node in the list.
******************************************************************************
* Function Calls:
*  LLFindLast		See above...
*  LLCutNode		See above...
*****************************************************************************/
int * LLPop(int * current)		   // Remove node from end of list
{
  int *last;
  
  last = LLFindLast(current);
  
  return LLCutNode(last);
}

/*****************************************************************************
* Function:  LLShift()
******************************************************************************
* Description:
*  Pops (removes) a node from the beginning of the list.  Returns the address
*  of the node we cut off.
******************************************************************************
* Function Calls:
*  LLFindFirst		See above...
*  LLCutNode		See above...
*****************************************************************************/
int * LLShift(int * current)	     	   // Remove node from start of list
{
  int *begin;
  
  begin = LLFindFirst(current);
  
  return LLCutNode(begin);
}

/*****************************************************************************
* Function:  LLUnshift()
******************************************************************************
* Description:
*  Inserts a node at the beginning of the list.
******************************************************************************
* Function Calls:
*  LLFindFirst		See above...
*  LLInsertNode		See above...
*****************************************************************************/
int * LLUnshift(int * current, int * add) // Add node to beginning of list
{
  int *begin;
  
  begin = LLFindFirst(current);
  
  return LLInsertNode(begin, add);
}

/*****************************************************************************
* Function:  LLSwapNodes()
******************************************************************************
* Description:
*  Switches two nodes' positions in the linked list.  It can handle cases
*  when the nodes are complete seperate, cases when the nodes are each
*  other's neighbors, and cases when the two nodes are the same one.
*  No return value.
******************************************************************************
* Function Calls:
*  LLFindNext		See Above...
*  LLFindPrev		See above...
*****************************************************************************/
void  LLSwapNodes(int * one, int * two)   // Switch two nodes' positions...
{
  int *firstprev, *firstnext;
  int *secondprev, *secondnext;
  
  if(one == two) return;	// if they're the same, do nothing
  
  firstprev  = LLFindPrev(one);	// Store the addresses of their neighbors...
  firstnext  = LLFindNext(one);
  secondprev = LLFindPrev(two);
  secondnext = LLFindNext(two);
  
  if(firstprev  != NULL)  (int *)firstprev[0] = two;	// Swap their
  if(firstnext  != NULL)  (int *)firstnext[1] = two;	// Neighbors' pointers
  if(secondprev != NULL)  (int *)secondprev[0] = one;
  if(secondnext != NULL)  (int *)secondnext[1] = one;
  
  (int *)one[0] = secondnext;	// Swap the two nodes' pointers
  (int *)one[1] = secondprev;
  (int *)two[0] = firstnext;
  (int *)two[1] = firstprev;
  
  if(firstnext  == two) (int *)one[1] = two;	// Fix things if they were
  if(firstprev  == two) (int *)one[0] = two;	// next to each other...
  if(secondprev == one) (int *)two[0] = one;
  if(secondnext == one) (int *)two[1] = one;
}

/*****************************************************************************
* Function:  LLCountNodes()
******************************************************************************
* Description:
*  Traverses the entire list and returns the number of nodes it finds.
******************************************************************************
* Function Calls:
*  LLFindFirst		See above...
*  LLFindNext		See above...
*****************************************************************************/
int   LLCountNodes(int * current)	// Returns # of nodes in entire list
{
  int numnodes=1;
  int *first, *next;
  
  first = LLFindFirst(current);		// Get the first node...
  next = first;
  

  // If next[0] (pointer to next node) is NULL, we hit the end.
  // If next[0] is the same as current, we've got a circular linked list...
  while((int *)next[0] != NULL  &&  (int *)next[0] != first)
  {
    numnodes++;
    next = LLFindNext(next);
  }
  
  return numnodes;
}


/*****************************************************************************
* Function:  LLSelectSort()
******************************************************************************
* Description:
*  Given any node in a list, sorts the whole list using a selection sort
*  algorithm.  You must supply a function to compare two nodes.  The
*  return value is the address of the new first node in the list.
******************************************************************************
* Function Calls:
*  LLCountNodes		See above...
*  LLFindLast		See above...
*  LLFindFirst		See above...
*  LLFindNext		See above...
*  LLFindPrev		See above...
*  LLSwapNodes		See above...
*  compare		The user-defined function which compares two nodes.
*			If you've ever used qsort, this should be familiar.
*****************************************************************************/
// Sorts the list and returns a pointer to the first node
int * LLSelectSort(int * current, int compare(void *, void *))
{
  int i,j;		// Junk / loop variables
  int numnodes;		// number of nodes in list
  int *best, *last;	// best match and last node in the list
  
  numnodes = LLCountNodes(current);	// get the number of nodes...
  last = LLFindLast(current);		// Find the last node.
  
  for(i=numnodes-1; i>1; i--)
  {
    current = LLFindFirst(current);	// get the first node again
    best = last;			// reset our "best" node
    
    for(j=0; j<i; j++)
    {
      if(compare(current, best) > 0)	// If we found a better match...
      {
        best = current;			// keep track of the "best" match
      }
      current = LLFindNext(current);	// Go to the next node.
    }

    LLSwapNodes(last, best);		// Switch two nodes...
    last = LLFindPrev(best);		// And go backwards by one node.
  }
  
  return LLFindFirst(current);		// return pointer to the first node.
}
