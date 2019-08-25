// [benchIO]
// 
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File time.c 
// Last Revision:   23th June, 2005
//
// time.c :
// 1) routines to measure times
// 2) routines to get differences of times
// 3) routines to use Linux RTC

#include "header/main.h"

//RTC system value
int rtc_default = RTC_FREQUENCY;
unsigned long rtc_hz_value = 0;

//System ticks
double clock_ticks = (double)0.0;
double time_resolution=(double)0;

//Global rtc file descriptor
//REMEMBER - RTC MUST BE USED ONLY IN SINGLE PROCESS/THREAD MODE!
int fd;

//get system max resolution
void get_resolution(int mode){

	struct time_elapsed starttime;
	struct time_elapsed finishtime;
        double besttime = 0;
        long  j,delay;
	int k;

        get_timing(mode,&starttime); 		// Warm up the instruction cache
        get_timing(mode,&finishtime);  		// Warm up the instruction cache
        delay=j=0;                 		// Warm up the data cache
	for(k=0;k<10;k++)
	{
	        while(1)
       	 	{
       	         	get_timing(mode,&starttime);
       	         	for(j=0;j< delay;j++)
       	                ;
       	         	get_timing(mode,&finishtime);
       	         	if(starttime.total==finishtime.total)
       	                 	delay++;
       	         	else
			{
				if(k==0)
					besttime=(finishtime.total-starttime.total);
				if((finishtime.total-starttime.total) < besttime)
					besttime=(finishtime.total-starttime.total);
                       	 	break;
			}
		}
        }
	time_resolution=besttime/1000000.0;
	#ifdef DEBUG
	fprintf(stderr,"\n\n[MAX TIME RESOLUTION] %f\n\n",time_resolution);
	#endif
}

//Measurement function
//INPUT: 
// 0 - gettimeofday
// 1 - times
// 2 - rtc (ONLY LINUX) - REMEMBER TO USE ioctl call!
//REMEMBER - RTC MUST BE USED ONLY IN SINGLE PROCESS/THREAD MODE!
void get_timing(int mode, struct time_elapsed *data_struct){

	struct tms tms_str;
	struct rusage ru;
	struct timeval tp;
//For Linux
#ifdef LINUX	
	int retval;
	unsigned long data;
#endif	
	int ex=0;
//For Cygwin
#ifdef WINDOWS
	LARGE_INTEGER freq,counter;
	double bigcounter;
#endif

	if(clock_ticks==0){
		//find value with this system call
		clock_ticks=(double)sysconf(_SC_CLK_TCK);
	}
	
	//store data!
	if(mode==0){
#ifdef WINDOWS
		// VERY IMPORTANT 
		// For Windows the time_of_day() is useless. It increments in 55 milli second   
		// increments. By using the Win32api one can get access to the high performance 
		// measurement interfaces. With this one can get back into the 8 to 9 microsecond resolution.
			
		QueryPerformanceFrequency(&freq);
        	QueryPerformanceCounter(&counter);
        	bigcounter=(double)counter.HighPart *(double)0xffffffff + (double)counter.LowPart;
        	data_struct->total = (double)(bigcounter/(double)freq.LowPart);
#else
		//USE gettimeofday (mode 0, default)
		ex=gettimeofday(&tp,(struct timezone *)NULL);
		if ( ex == -1){
			//Managing Error
			fprintf(stderr, "gettimeofday error.\n");
			}
		data_struct->total=(double)((((double)tp.tv_sec)+(((double) tp.tv_usec) * 0.000001)));
#endif
	}
	else if(mode==1){
		//USE times.h (mode 1)
		data_struct->total=(double)((double)times(&tms_str)/(double)clock_ticks);
	}
#ifdef LINUX
	else if(mode==2){
		//USE Real time clock (mode 2 only for M1, only for linux)
		/* This blocks */
		retval = read(fd, &data, sizeof(unsigned long));
		if (retval == -1) {
			//Managing Error
			fprintf(stderr, "error reading /dev/rtc value: is it setted with IOCTL?\n");
			exit(errno);
		}
		
		data=data>>8;
		data_struct->total=(double)(data/(double)rtc_hz_value);
	}
#endif
	else{	
		//Managing argc error
		fprintf(stderr, "You have selected an unimplemented measurement mode: use gettimeofday.\n");
		//so we use Mode 1 gettimeofday
		if (gettimeofday(&tp, (struct timezone *) NULL) == -1){
			//Managing Error
			fprintf(stderr, "gettimeofday error.\n");
			}
		data_struct->total=(double)((((double)tp.tv_sec)+(((double) tp.tv_usec) * 0.000001)));
	}
	
	//gets statistics with getrusage
	if (getrusage (RUSAGE_SELF, &ru)){
		perror ("getrusage");
		}
	//and stores data
	//the USER time measurements
	data_struct->user_time=(double)(ru.ru_utime.tv_sec+.000001 * ru.ru_utime.tv_usec);
	
	//the SYSTEM time measurements
	data_struct->system_time=(double)(ru.ru_stime.tv_sec+.000001 * ru.ru_stime.tv_usec);
	
	//CPU utilization measurements
	data_struct->cpu_usage=(float)0;
		
		
		#ifdef DEBUG
			//Verbosely Debug
			if(mode==1){
				fprintf(stderr,"CLOCK TC: %f\n",clock_ticks);
				}
			fprintf(stderr,"TOTAL: %f\n",data_struct->total);
			fprintf(stderr,"USER: %f\n",data_struct->user_time);
			fprintf(stderr,"SYSTEM: %f\n",data_struct->system_time);
			fprintf(stderr,"-----------------------\n");
		#endif
		
		
			
	return;
}

//This function calculates the DELTA of timings (differences between start time and finish time)
void get_delta(struct time_elapsed *OLD,struct time_elapsed *NEW,struct time_elapsed *DELTA,int mode){

	double min_bound;

	if(time_resolution==(double)0){
		min_bound=(double)0.000001;
	}else{
		min_bound=time_resolution;
	}

	DELTA->total=(double)((NEW->total)-(OLD->total));
	//if value is 0, set to min resolution
	if(mode==2 && DELTA->total==(double)0) DELTA->total = min_bound;
	if(mode==1 && DELTA->total==(double)0) DELTA->total = min_bound;
	if(mode==2 && DELTA->total<(double)0) DELTA->total = min_bound;
	if(mode==1 && DELTA->total<(double)0) DELTA->total = min_bound;
	DELTA->user_time=(double)((NEW->user_time)-(OLD->user_time));
	DELTA->system_time=(double)((NEW->system_time)-(OLD->system_time));
	DELTA->cpu_usage=(float)((DELTA->system_time+DELTA->user_time)/DELTA->total)*(float)100;
	if(DELTA->cpu_usage>(float)100) DELTA->cpu_usage=(float)100;
	if(DELTA->cpu_usage<(float)0) DELTA->cpu_usage=(float)0;
	return;
}

#ifdef LINUX
//This function enable the RTC clock. It also sets device frequency
int system_rtc_set(unsigned long freq){
	unsigned long tmp;
	int retval;
	
	if(getuid()!=0){
		fprintf(stderr, "YOU MUST BE ROOT TO USE RTC.\n");
		exit(-1);
	}
	if(freq<=(unsigned long)0 || freq>(unsigned long)8192){
		fprintf(stderr, "frequency selected is out of range. Set default (256Hz).\n");
		tmp=(unsigned long)rtc_default;
		rtc_hz_value=(unsigned long)rtc_default;
	}else{
		tmp=(unsigned long)freq;
		rtc_hz_value=(unsigned long)freq;
	}
		
	fd = open ("/dev/rtc", O_RDONLY);
	
	retval = ioctl(fd, RTC_IRQP_SET, tmp);
	if (retval == -1) {
		fprintf(stderr, "ioctl error.\n");
		exit(errno);
	}
	#ifdef DEBUG
	//Verbosely Debug	
	retval = ioctl(fd, RTC_IRQP_READ, tmp);
		if (retval == -1) {
			fprintf(stderr, "ioctl error 2.\n");
			exit(errno);
		}
		fprintf(stderr, "RTC is at %ld Hz.\n", tmp);
		fflush(stderr);
	#endif
	// Enable periodic interrupts 
	retval = ioctl(fd, RTC_PIE_ON, 0);
	if (retval == -1) {
		fprintf(stderr, "ioctl error 3.\n");
		exit(errno);
	}
	
	
	return 1;
}

//This function disable RTC clock and close /dev/rtc
void system_rtc_down(){
	
	int retval;
	
	// Disable periodic interrupts
	retval = ioctl(fd, RTC_PIE_OFF, 0);
	if (retval == -1) {
		fprintf(stderr, "ioctl close_rtc error.\n");
		exit(errno);
	}
	
	close(fd);
}

#endif


