// [benchIO] HEADER FILE
//
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File thread.h
// Last Revision:   23th June, 2005

#include "main.h"

//Data Structure for Thread Parameters
struct thread_params{
	int mode;
	unsigned long int filesize;
	unsigned long int blocksize;
	int num_thr_pro;
	int process;
	int thread;
	char* mount_point;
	int gettimes;
	int o_direct;
	int o_sync;
	int posix_rw;
	int include_flush;
	int warmup;
	int sem_ident;
	int* cond_var;
	pthread_cond_t *thread_flag_cv;
	pthread_mutex_t *thread_flag_mutex;
};

//Data Structure for Time Measures
struct test_result{
	double swt;
	double swu;
	double sws;
	float swc;
	double rwt;
	double rwu;
	double rws;
	float rwc;
	double srt;
	double sru;
	double srs;
	float src;
	double rrt;
	double rru;
	double rrs;
	float rrc;
};

void* thread_function(void* params);
