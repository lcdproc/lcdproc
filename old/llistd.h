#ifndef LLISTD_H
#define LLISTD_H
/*****************************************************************************
* AUTHOR: Scott Scriven, SSN: 523-21-9640, CLASS: CS151.003, DATE: 96-11-21
******************************************************************************
* Doubly Linked Lists!
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


// See llistd.c for detailed descriptions of these functions.

int * LLMakeFirstNode(int * first);	// Make a standalone node...

int * LLFindFirst(int * current);	// returns address of First node
int * LLFindNext(int * current);	// returns address of next node
int * LLFindPrev(int * current);	// returns address of prev node
int * LLFindLast(int * current);	// returns address of last node

int * LLAddNode(int * current, int * add);	// Adds node AFTER current one
int * LLInsertNode(int * current, int * add);	// Adds node BEFORE current one
int * LLCutNode(int * cut);			// Removes a node from the link

int * LLPush(int * current, int * add);	   // Add node to end of list
int * LLPop(int * current);		   // Remove node from end of list
int * LLShift(int * current);	     	   // Remove node from start of list
int * LLUnshift(int * current, int * add); // Add node to beginning of list

void  LLSwapNodes(int * one, int * two);   // Switch two nodes positions...

int   LLCountNodes(int * current);	// Returns # of nodes in entire list

// "Selection Sort"s the list and returns a pointer to the first node
int * LLSelectSort(int * current, int compare(void *, void *));

#endif
