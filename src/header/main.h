// [benchIO] MAIN HEADER FILE
//
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File main.h
// Last Revision:   23th June, 2005

//System Includes
#pragma once
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <getopt.h>
#include <regex.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <sys/vfs.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/ioctl.h>
#ifdef LINUX
	#include <linux/rtc.h>
	// O_DIRECT (Linux only)
	#define __USE_GNU
#endif
#include <fcntl.h>
#ifdef WINDOWS
	#include <Windows.h>
#endif

//Data structure for Options
struct options{
	int verbose;
	char* mount_point;
	int mode;
	int number;
	int maxfsize;
	char* outfile;
	int gettimes;
	int o_direct;
	int o_sync;
	int posix_rw;
	int vfs;
	int dirty;
	int prsched;
	int priority;
	int iosched;
	int rtcfreq;
	int include_flush;
	int lock_memory;
	int warmup;
};

//BenchIO Includes
#include "process.h"
#include "process_semaphore.h"
#include "thread.h"
#include "thread_warmup.h"
#include "thread_semaphore.h"
#include "output.h"
#include "utils.h"
#include "time.h"
#ifdef LINUX
	#include "linux.h"
#endif

//BenchIO Defines
#define _GNU_SOURCE
#define RTC_FREQUENCY 8192
#define DEFAULT_NUM_PT 10
#define DEFAULT_FILENAME "benchIO"
#define CSVFILE DEFAULT_FILENAME".csv"
#define RESDIR "results"
#define RESFILE DEFAULT_FILENAME

// Base unit is Kb

// 4K <= Filesize <= 2G

// 4K
#define FILESIZE_MIN 4
// 512M
#define FILESIZE_MAX 512*1024
// 4M
#define FILESIZE_MAX_MULTI 4*1024
// 2G
#define FILESIZE_LIMIT 6*1024*1024

// 4K <= Blocksize <= 16M

// 4K
#define BLOCKSIZE_MIN 4
// 16M
#define BLOCKSIZE_MAX 16*1024
// 4K
#define BLOCKSIZE_MAX_MULTI 4
