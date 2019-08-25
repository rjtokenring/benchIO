// [benchIO]
// 
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File process_semaphore.c 
// Last Revision:   23th June, 2005
//
// process_semaphore.c file :
// 1) routines to manage process semaphores

//Include header
#ifdef WINDOWS
	//noop - cygwin doesn't support this kind of semaphores.
#else
#include "header/main.h"


//This function allocate the binary semaphore
int binary_semaphore_allocation(key_t key, int sem_flags){
	//key IPC_PRIVATE, sem_flags O_CREAT | S_IRWXU
	return semget (key, 1, sem_flags);
}

//This function deallocate the binary semaphore
int binary_semaphore_deallocate(int semid){
  union semun ignored_argument;
  return semctl (semid, 1, IPC_RMID, ignored_argument);
}
//This function inizialize the binary semaphore
int binary_semaphore_initialize(int semid,int proc_numba){
  union semun argument;
  unsigned short values[1];
  values[0] = (int) proc_numba;
  argument.array = values;
  return semctl(semid, 0, SETALL, argument);
}

//This function implements the wait in the binary semaphore
int binary_semaphore_wait(int semid){
  struct sembuf operations[1];
  // Use the first (and only) semaphore. 
  operations[0].sem_num = 0;
  // Decrement by 1
  operations[0].sem_op = -1;
  // Permit UNDO.
  operations[0].sem_flg = SEM_UNDO;
  return semop (semid, operations, 1);
}

//This function blocks until semaphore valure reach ZERO
int binary_semaphore_wait_zero(int semid){
  struct sembuf operations[1];
  operations[0].sem_num = 0;
  operations[0].sem_op = 0;
  operations[0].sem_flg = SEM_UNDO;
  return semop (semid, operations, 1);
}

//This function implements the post in the binary semaphore
int binary_semaphore_post(int semid){
  struct sembuf operations[1];
  // Use the first (and only) semaphore.
  operations[0].sem_num = 0;
  // Increment by 1.
  operations[0].sem_op = 1;
  // Permit UNDO
  operations[0].sem_flg = SEM_UNDO;
  return semop (semid, operations, 1);
}

#endif

