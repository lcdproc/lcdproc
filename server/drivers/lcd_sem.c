/*
 * lcd_sem.c  -- semaphore code written for lcdtime and meter to mediate
 * access to the parallel port.
 * 
 * Written by Benjamin Tse (blt@mundil.cs.mu.edu.au); August,October 1995
 * 
 * Functions in this file:
 *   getkey      returns the key for the semaphore
 *   sem_get     create the semaphore (and initialise) if one doesn't exist
 *                otherwise return its key
 *   sem_wait    wait on the semaphore
 *   sem_signal  signal on the semaphore
 *   sem_remove  remove the semaphore
 * 
 * Legal stuff: At no stage was this program written, assembled or compiled on
 * any computer at the University of Melbourne, Australia. This program is
 * Copyright (C) 1995 Benjamin Tse (blt@mundil.cs.mu.oz.au) and covered by
 * GNU's GPL. In particular, this program is free software and comes WITHOUT
 * ANY WARRANTY.
 * 
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>	/* for semaphore functions */
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include "lcd_sem.h"

#define SEMAPHORE       "portctrl"
#define SEMKEY          0x706f7274      /* semaphore key */
#define SEMCOUNT        1               /* number of semaphores to create */
#define WMODE           0660		/* access permissions */

#define SEM_SIGNAL      0,1,SEM_UNDO
#define SEM_WAIT        0,-1,SEM_UNDO

/* functions local to this file */
static key_t getkey(register char *p);
 
/* global variables */
static struct sembuf semaphore_wait = {SEM_WAIT};
static struct sembuf semaphore_signal = {SEM_SIGNAL};

static char rcsId[] = "$Id$";

/*
 * getkey  returns the key for the semaphore
 */

static key_t
getkey(register char *p)
{
    return((key_t) SEMKEY);
}

/*
 * create the semaphore if one doesn't exist and initialise it to 1, else 
 * return the semaphore set id
 */

int
sem_get(void)
{
    int semid;
    union semun semval;

    if ((semid = semget(getkey(SEMAPHORE), SEMCOUNT, 
			IPC_CREAT | IPC_EXCL | WMODE)) < 0)
    {
	switch (errno)
	{
	    case EEXIST:
	    	/* semaphore set exists, get id and return it */
		if ((semid = semget(getkey(SEMAPHORE), SEMCOUNT,
		    		IPC_EXCL | WMODE)) < 0)
		{
		    perror("semget");
		    exit(1);
		}
		return semid;
		break;
	    case EACCES:
	    	/* don't have permissions for semaphore, need to change key */
		perror("semget, can't get permissions for semaphore");
		exit(1);
		break;
	    default:
		perror("semget");
	    	exit(1);
		break;
	}
    }
    else
    {
	/* initialise semaphore to 1 */
    	semval.val = 1;

	if (semctl(semid, 0, SETVAL, semval) < 0)
	{
	    perror("setval, can't initialise semaphore");
	    exit(1);
	}
    }
								        
    return semid;
}

/*
 * wait on the semaphore
 */

int
sem_wait(int sid)
{
    if (semop(sid, &semaphore_wait, 1) < -1)
    {
	perror(SEMAPHORE);
	exit(1);
    }

    return 0;
}

/*
 * signal on the semaphore
 */

int
sem_signal(int sid)
{
    if (semop(sid, &semaphore_signal, 1) < -1)
    {
	perror(SEMAPHORE);
	exit(1);
    }

    return 0;
}

/*
 * remove the semaphore
 */

int
sem_remove(int sid)
{
    int i;
    union semun dummy;
    
    if ((i = semctl(sid, 0, IPC_RMID, dummy)) < 0)
    {
	switch(i)
	{
	    case EIDRM:
	        /* semaphore removed */
	        return 0;
	        break;
	    default:
	        perror("semctl, removing semaphore");
	        exit(1);
	}
    }

    return 0;
}

