// [benchIO]
// 
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File utils.c 
// Last Revision:   23th June, 2005
//
// utils.c :
// 1) helper routines: check if uid=root, get a line from a file, check mount point path, set process scheduler

#include "header/main.h"

#define OVERFLOW 150


//This function checks if I AM ROOT
//INPUT: void
//OUTPUT: void
int I_am_root(){
	if(getuid()!=0){
		fprintf(stderr, "[EE] you MUST be ROOT! exiting.\n");
		return 0;
	}
	return 1;
}

//This function gets a line from a specified file
//INPUT: standard FILE pointer
char *get_line_from_file(FILE *file){

	static const int GROWBY = 80;
	int ch;
	int idx = 0;
	char *linebuf = NULL;
	int linebufsz = 0;

	while (1) {
		ch = fgetc(file);
		if (ch == EOF)
			break;
		/* grow the line buffer as necessary */
		if (idx > linebufsz-2)
			linebuf = realloc(linebuf, linebufsz += GROWBY);
		linebuf[idx++] = (char)ch;
		if ((char)ch == '\n')
			break;
	}

	if (idx == 0)
		return NULL;

	linebuf[idx] = 0;
	return linebuf;
}


//This function checks if mount is well formed
//INPUT: mount point (char *)
//OUTPUT: validated_mount_point (char* - pointer)
char *mnt_sanity_check(char *mount){

	char escape='/', temp,temp_incipit, last;
	char *proc, *sys, *dev, *etc, *part;
	int i=0;
	
		//if mount is empty...
		if(strlen(mount)==0){
			fprintf(stderr, "[EE] the mount point string is invalid.\n");
			exit(-1);
		}
	
	proc=(char*)malloc(5);
	sys=(char*)malloc(5);
	dev=(char*)malloc(5);
	etc=(char*)malloc(5);
	part=(char*)malloc(5);
	
	//estract the first 4 char of the mount point
	for(i=0;i<4;i++){
		part[i]=mount[i];
	}
	part[4]='\0';
	
	sprintf(proc,"/pro");
	sprintf(sys,"/sys");
	sprintf(dev,"/dev");
	sprintf(etc,"/etc");
	
	//if it not starts with / or it's too long
	temp_incipit=mount[0];
	last=mount[1];
	temp=mount[(strlen(mount)-1)];
		if(temp_incipit!=escape || strlen(mount)>OVERFLOW){
			fprintf(stderr, "[EE] the mount point path is malformed.\n");
			free(proc);
			free(sys);
			free(etc);
			free(dev);
			free(part);
			exit(-1);
		}
	//If it finish with /
	if(temp==escape){
		mount[(strlen(mount)-1)]='\0';
	}
	
	//if path is /proc or /sys or /dev or /etc
	if(!strcmp(proc,part) || !strcmp(sys,part) || !strcmp(dev,part) || !strcmp(etc,part)){
		fprintf(stderr, "[EE] you can't make test on /proc,/sys,/dev,/etc.\n");
		free(proc);
		free(sys);
		free(etc);
		free(dev);
		free(part);
		exit(-1);
	}
	
	//If path is root (/) filesystem.. stop.
	if(temp_incipit==escape && last=='\0'){
		fprintf(stderr, "[EE] you can't make test on root directory /: it's not safe!\n");
		free(proc);
		free(sys);
		free(etc);
		free(dev);
		free(part);
		exit(-1);
	}
	
	//freeing unused memory
	free(proc);
	free(sys);
	free(etc);
	free(dev);
	free(part);
	//and then return...
	return mount;
}


//It changes the process scheduler...
//INPUT: int with value 
// 1 - SCHED_RR
// 2 - SCHED_FIFO
// 0 - SCHED_OTHER
//OUTPUT void
//in case of errors, it blocks program execution.
void set_process_sched(int set_value, int prio){

	struct sched_param parms;
	int prio_max, prio_min;
	
	//Root checks
	if(!I_am_root()){
		fprintf(stderr,"[EE] can't set process scheduler.\n");
		return;
	}
	
	if(set_value<0 || set_value>2){
		//scheduler policy is NOT correct!
		fprintf(stderr, "[EE] scheduler policy is NOT correct!\n");
		exit(-1);
	}
	
	//getting valid priority range for this system (POSIX compliant)
	prio_min=sched_get_priority_min(set_value);
	prio_max=sched_get_priority_max(set_value);
	
	if(prio>(prio_min-1) && prio<(prio_max+1)){
		//setting default process scheduler
		parms.sched_priority = prio;
		if(sched_setscheduler(0, set_value, &parms) < 0) {
			fprintf(stderr, "[EE] some problem with process scheduler.\n");
			exit(-1);
		}
		fprintf(stderr, "[I] process scheduler setted.\n");
		
	}else{
		fprintf(stderr,"[EE] priority range is out of bound. The range for this system is [%d - %d].\n",prio_min,prio_max);
		exit(-1);
	}
}



