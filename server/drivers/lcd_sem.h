/*
 * header file for lcd semaphore operations
 */

#ifndef LCD_SEM_H
#define LCD_SEM_H

int sem_get (void);
int sem_wait (int sid);
int sem_signal (int sid);
int sem_remove (int sid);

#endif
