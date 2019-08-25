// [benchIO] HEADER FILE
//
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File thread_semaphore.h
// Last Revision:   23th June, 2005

#include "main.h"

extern int thread_flag;
extern pthread_cond_t thread_flag_cv;
extern pthread_mutex_t thread_flag_mutex;

void initialize_flag();
void reset_flag();
void increment_thread_flag(struct thread_params* tp);


