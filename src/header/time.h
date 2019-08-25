// [benchIO] HEADER FILE
// 
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File time.h  
// Last Revision:   23th June, 2005

#include "main.h"

//Data Structure for times() Function
struct time_elapsed{
	double total;
	double user_time;
	double system_time;
	float cpu_usage;
};

void get_resolution(int mode);
void get_timing(int mode, struct time_elapsed *data_structure);
void get_delta(struct time_elapsed *OLD,struct time_elapsed *NEW,struct time_elapsed *DELTA,int mode);
int system_rtc_set(unsigned long freq);
void system_rtc_down();


