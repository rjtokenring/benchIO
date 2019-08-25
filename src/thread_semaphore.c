// [benchIO]
//
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File thread_semaphore.c
// Last Revision:   23th June, 2005
//
// thread_semaphore.c :
// 1) routines to manage thread semaphores

//Includes header
#include "header/main.h"

//Setting thread flag to zero
int thread_flag = 0;
//Define the condition variable
pthread_cond_t thread_flag_cv;
//Define the thread mutex
pthread_mutex_t thread_flag_mutex;


//This function, when called, inizialize the flag for the thread semaphore
void initialize_flag()
{
	// Initialize the mutex and condition variable.
	pthread_mutex_init (&thread_flag_mutex, NULL);
	pthread_cond_init (&thread_flag_cv, NULL);
	// Initialize the flag value.
	thread_flag = 0;
}

//This function, when called, reset the flag to zero with the necessary operations (lock/unlock)
void reset_flag()
{
	// Lock the mutex before accessing the flag value
	pthread_mutex_lock(&thread_flag_mutex);
	// Initialize the flag value.
	thread_flag = 0;
	// Unlock the mutex before go on
	pthread_mutex_unlock(&thread_flag_mutex);
}

//This function, when called, increments the flag with the necessary operations (lock/unlock)
void increment_thread_flag(struct thread_params* tp)
{
	// Lock the mutex before accessing the flag value
	if(pthread_mutex_lock(tp->thread_flag_mutex)<0)
		//Managing Error
		fprintf(stderr,"increment_thread_flag mutex 1 error.\n");
	//Increment flag
	thread_flag = thread_flag+1;
	pthread_cond_signal(tp->thread_flag_cv);

	// Unlock the mutex before go on
	if(pthread_mutex_unlock(tp->thread_flag_mutex)<0)
		//Managing Error	
		fprintf(stderr,"increment_thread_flag mutex 2 error.\n");
}

