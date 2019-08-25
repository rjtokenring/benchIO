// [benchIO] HEADER FILE
//
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File process_semaphore.h
// Last Revision:   23th June, 2005

#include "main.h"

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};

int binary_semaphore_allocation(key_t key, int sem_flags);
int binary_semaphore_deallocate(int semid);
int binary_semaphore_deallocate(int semid);
int binary_semaphore_initialize(int semid,int proc_numba);
int binary_semaphore_wait(int semid);
int binary_semaphore_wait_zero(int semid);
int binary_semaphore_post(int semid);


