// [benchIO]
//
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File linux.c
// Last Revision:   23th June, 2005
//
// linux.c :
// 1) setting Linux-specific features

#include "header/main.h"

#ifdef LINUX

#define OVERFLOW 150

//Defining Global Variables
//Defining the var to contain the device name
char* device;
//Defining the struct to manage software raid. The struct contains all the the device that compose the SWRAID
struct md{
	char *dev;
	struct md *next;
};
struct md *md_queue=NULL;

int is_raid=0;
int is_lvm=0;
int is_network=0;

//Defining the var to indicate the Scheduler for the device
char* sched;
//The same above to manage software raid, but this time with setted scheduler.
struct md_sched{
	char *sched;
	char *dev;
	struct md_sched *next;
};
struct md_sched *md_sched_queue=NULL;

//This var defines the dirty background ration and the VFS chahe pressure.
char *dirty_background_ratio, *vfs_cache_pressure;


//This function is a line parser that match a given pattern (the pattern is a string)
char *parse_line(char* char_line){

	int i,h=0,lenght=0,flag=0;
	char carat_in='[' , carat_out=']' , *output;
	
	output=(char*)malloc(15);
	
	lenght=(strlen(char_line)+1);
	for(i=0;i<lenght;i++){
		if(char_line[i]==carat_out) {
			output[h]='\0';
			break;
			}
		if(flag==1){
		 	output[h]=char_line[i];
		 	h++;
			if(h>15){
				fprintf(stderr, "[EE] parse_line: possible overflow.\n");
				exit(-1);
			}
		 }
		if(char_line[i]==carat_in) {flag=1;}
	}
	return (char*)output;
}

//This function is a line parser from /proc/mdstat and sets devices associated to a SWRAID (MD) dev (char *md var)
void mdstat_parse_line(char* dev){

	int i,lenght=0, find=0, reflags;
	char carat_in='[', *temp, *char_line;
	struct md *new;
	static regex_t regex;
	FILE *fd;
	//Verbosely Debugging
	#ifdef DEBUG
		fprintf(stderr,"[+] mdstat_parse_line - dev var= %s\n",dev);
	#endif
		
	//fopen the /proc/mdstat file
	fd = (FILE *)fopen("/proc/mdstat", "r");
		if(fd==NULL){
			//Fault in fopen management, exits the function returning error code -1 
			fprintf(stderr, "[EE] some problem opening /proc/mdstat.\n");
			exit(-1);
		}

	//The REGEX pattern matchmaking engine

	//Setting up REGEX parameters used in file searching.
	reflags = REG_NOSUB | REG_NEWLINE;
	//Compile POSIX REGEX
	if (regcomp(&regex, dev, reflags) != 0){
		//Fault in REGEX management, exits the function returning error code -1 
		fprintf(stderr, "[EE] some problem with regular expression.\n");
		exit(-1);
		}
		
	//while fd!=EOF - finding the pattern
	while((char_line=get_line_from_file(fd))!=NULL){
	//Verbosely Debugging
		#ifdef DEBUG
			fprintf(stderr,"[+] mdstat_parse_line: lines= %s.\n",char_line);
		#endif
		//get a new line from /proc/mdstat
		//POSIX REGEXec manages to find the pattern
		if(regexec(&regex, char_line, 0, NULL, 0)==0){
		//This means that the pattern is found
			find=1;
			break;
		}
		free(char_line);
	}
	
	//Flushing and fclosing the /proc/mdstat
	fflush(fd);
	fclose(fd);
	
	if(find==0){
		//This is the case in which no results are founded
		fprintf(stderr, "[EE] the raid device specified is not present in mount table.\n");
		exit(-1);
	}
		
	lenght=(strlen(char_line)+1);
	for(i=0;i<lenght;i++){
		if(char_line[i]==carat_in) {
			//init space
			new=(struct md*)malloc(sizeof(struct md));
			temp=(char*)malloc(4);
			temp[0]=char_line[i-4];
			temp[1]=char_line[i-3];
			temp[2]=char_line[i-2];
			temp[3]='\0';
	//Verbosely Debugging
			#ifdef DEBUG
				fprintf(stderr,"MD DEVs: %s.\n", temp);
			#endif
			new->dev=temp;
			new->next=md_queue;
			md_queue=new;
			}
	}
	//Verbosely Debugging
	#ifdef DEBUG
		new=md_queue;
		do{			
			fprintf(stderr,"MD_QUEUE DEVs: %s.\n", new->dev);
			new=new->next;
		}while(new!=NULL);
	#endif
	//free unused memory
	free(char_line);
	//setting RAID int;
	is_raid=1;
}

//This function is a line parser from "pvscan" output. Then it sets devices associated to a LVM
void pvscan_parse_line(FILE *fd){

	int i, find=0, reflags;
	char carat_in='V', *temp, *char_line, *find_me, *raid, *raid_found;
	struct md *new;
	static regex_t regex;
	
	//Checks file pointer in pvscan_parse_line if is not initialized.
	if(fd==NULL){
		fprintf(stderr, "[EE] file pointer in pvscan_parse_line is not initialized.\n");
		pclose(fd);
		exit(-1);
	}
	
	find_me=(char*)malloc(9);
	sprintf(find_me," PV /dev");
	//setting REGEX parameters. It is used in file searching
	reflags = REG_NOSUB | REG_NEWLINE | REG_ICASE;
	//compile POSIX regex
	if (regcomp(&regex, find_me, reflags) != 0){
		//Managing error
		fprintf(stderr, "[EE] pvscan_parse_line: some problem with regular expression.\n");
		free(find_me);
		pclose(fd);
		exit(-1);
		}
	
	free(find_me);
	
	//setting var
	raid=(char*)malloc(3);
	raid_found=(char*)malloc(3);
	sprintf(raid,"md");
		
	//while fd!=EOF - finding the pattern
	while((char_line=get_line_from_file(fd))!=NULL){
		//get a new line from pvscan pipe
		//using POSIX regexec to find the line
		if(regexec(&regex, char_line, 0, NULL, 0)==0){
			//Pattern found case: we could have multiple devices used in the LVM
			find=1;
			//Building the data structure			
			i=3;
			if(char_line[i]==carat_in) {
			//we are where is the PV word
				temp=(char*)malloc(5);
				temp[0]=char_line[i+7];
				temp[1]=char_line[i+8];
				temp[2]=char_line[i+9];
				temp[3]='\0';
					raid_found[0]=char_line[i+7];
					raid_found[1]=char_line[i+8];
					raid_found[2]='\0';
					//if it is a RAID....
					if(!strcmp(raid_found,raid)){
						//Verbosely Debugging
						#ifdef DEBUG
							fprintf(stderr,"[+] LVM RAID FOUND - starting mdstat_parse_line: %s.\n", temp);
						#endif
						//call mdstat parser to find devs connetted to RAID
						 mdstat_parse_line(temp);
					}else{
						//Adding devs to the queue
						new=(struct md*)malloc(sizeof(struct md)+1);
						new->dev=temp;
						new->next=md_queue;
						md_queue=new;
					}
				//Verbosely Debugging
				#ifdef DEBUG
					fprintf(stderr,"[+] LVM FIND: %s.\n", temp);
				#endif
			}
		}
	}
	
	if(find==0){
		//Managing Errors
		fprintf(stderr, "[EE] the lvm device specified is not present in pvscan output.\n");
		pclose(fd);
		exit(-1);
	}
	//Verbosely Debugging
	#ifdef DEBUG
		new=md_queue;
		do{			
			fprintf(stderr,"[+] LVM_QUEUE DEVs in list: %s.\n", new->dev);
			new=new->next;
		}while(new!=NULL);
	#endif
	
	//free unused memory
	free(char_line);
	free(raid);
	free(raid_found);
	//setting LVM int;
	is_lvm=1;
	//setting LVM -> device
	device=(char*)malloc(4);
	sprintf(device,"lvm");
}

//This function finds devices used by the LVM 
void lvm_get_dev(void){

	FILE *stream;
	char *mess;
	
	//Preliminary Root Checks
	if(!I_am_root()){
	// Managing error
		fprintf(stderr,"[EE] Can't use pvscan.\n");
		return;
	}
	
	//Invoking external program "pvscan"
	stream=popen("pvscan","r");
		if(stream==NULL){
			//Managing Errors
			fprintf(stderr, "[EE] some problems opening program pvscan: it is present or it is reachable? Try to control your $PATH.\n");
			exit(-1);
		}
	//Getting info from the pipe
	pvscan_parse_line(stream);
	pclose(stream);
	//Invoking external program "lvscan"
	stream=popen("lvscan","r");
		//Managing Errors
		if(stream==NULL){
			fprintf(stderr, "[EE] some problems opening program lvscan: it is present or it is reachable? Try to control your $PATH.\n");
			exit(-1);
		}
	mess=get_line_from_file(stream);
	//Printing out errors
	fprintf(stderr, "[I] LVM - Active Logical Volumes:\n");
	fprintf(stderr, mess);
	pclose(stream);
	free(mess);
	
}

//This function finds the device (output without /dev ) of a defined mount point
//It inits global var 'device' (and call malloc on it)
//INPUT: mount point (eg. /mnt/hda)
char *get_dev_by_mnt(char *mount_point){

	static regex_t regex;
	int reflags, find=0, i=0, t;
	char *line, *dest;
	FILE *fd;
	
	//Open the /proc/mounts
	fd = (FILE *)fopen("/proc/mounts", "r");
		if(fd==NULL){
			//something is broken...
			fprintf(stderr, "[EE] some problem opening proc filesystem: it is mounted?\n");
			exit(-1);
		}
	
	//Check sanity on mount_point
	mount_point=(char *)mnt_sanity_check(mount_point);
	
	//getting FILESYSTEM INFO
	//get_filesystem_info(mount_point);	
	
	//Setting regex params. It is used in file searching
	reflags = REG_NOSUB | REG_NEWLINE | REG_ICASE;
	//Compiling POSIX regex
	if (regcomp(&regex, mount_point, reflags) != 0){
		//something is broken...
		fprintf(stderr, "[EE] some problem with regular expression.\n");
		exit(-1);
		}
		
	//while fd!=EOF - finding the pattern
	while((line=get_line_from_file(fd))!=NULL){
		//get a new line from /proc/mounts
		//using POSIX regexec to find the line
		if(regexec(&regex, line, 0, NULL, 0)==0){
			//pattern found!
			find=1;
			break;
		}
		free(line);
	}
	
	//closing the file
	fflush(fd);
	fclose(fd);
	
	if(find==0){
		//Managin Error
		fprintf(stderr, "[EE] the path specified is not present in mount table.\n");
		exit(-1);	
	}
	//Verbosely Debugging
	#ifdef DEBUG
		fprintf(stderr,"[+] get_dev_by_mnt STRING - %s.\n",line);
	#endif
	
	//init dest
	dest=(char*)malloc(5);
	
	t=0;
	//finding the device	
	for(i=5;i<8;i++){
		if(dest[0]=='f' && dest[1]=='d'){
			dest[t]=line[i];
			t++;
			break;
		}
		if(dest[0]=='m' && dest[1]=='d'){
			dest[t]=line[i];
			t++;
			dest[t]='\0';
			//Setting le list of devs associated to mdX dev
			mdstat_parse_line(dest);
			break;
		}
		if((line[i])>='0' && (line[i])<='9')
			break;
		
		dest[t]=line[i];
		t++;
	}
	
	//Adding string terminator
	dest[t]='\0';
		
	//The device could be an LVM or a network File System:
		if(!(dest[0]=='f' || dest[0]=='h' || dest[0]=='s' || dest[0]=='m')){
			if(dest[0]=='.' || dest[0]=='\0' || dest[0]=='/' || dest[0]=='\\'){
				//it could be a network FS
				fprintf(stderr, "[I] it could be a network FS like CIFS or NFS.\n");
				is_network=1;
				//store value in device var
				device=(char *)malloc(8);
				snprintf(device,8,"network");
				//and return....
				return (char*)device;
				
			}else{
				//Verbosely Debugging
				#ifdef DEBUG
					fprintf(stderr,"[+] IT COULD BE A LVM DEVICE: %s\n",dest);
				#endif
			lvm_get_dev();
			}
		}else{
			//Verbosely Debugging
			#ifdef DEBUG
				fprintf(stderr,"[+] get_dev_by_mnt: AND THEN - %s.\n",dest);
			#endif
	
			//Storing values in the device var
			device=(char *)malloc(sizeof(dest)+1);
			strncpy(device,dest,sizeof(dest));
		}
	
	//freeing unused space
	free(line);
	free(dest);
	//Verbosely Debugging
	#ifdef DEBUG
		fprintf(stderr,"[+] get_dev_by_mnt: VAR DEVICE - %s.\n",device);
	#endif
	
	return (char*)device;
}

//This function get system env and information about the testing filesystem- it use 'get_dev_by_mnt', so it initialize  global variable'device'
//INPUT: mount point
//OUTPUT: void (it sets "sched" var)
void get_system_env(char *mount_point){

	char *dev, *path;
	char *lines=NULL, *temp;
	//struct statfs *struttura;
	FILE *fd;
	struct md *new;
	struct md_sched *temp_sched;

	if(!I_am_root()){
		fprintf(stderr,"[EE] Can't check system value.\n");
		return;
	}
	
	if(device==NULL){
		//check mount_point sanity
		mount_point=(char *)mnt_sanity_check(mount_point);
		dev=(char*)get_dev_by_mnt(mount_point);
	}
	
	md_sched_queue=NULL;
	
	//if it is raid or lvm... I could have multiple devs
	if(is_raid==1 || is_lvm==1){
		new=md_queue;
		path=(char*)malloc(32);
		while(new!=NULL){
			
			//build path..
			snprintf(path,31,"/sys/block/%s/queue/scheduler",new->dev);
			//Verbosely Debugging
			#ifdef DEBUG
				fprintf(stderr,"get_system_env DEVs: %s.\n", new->dev);
			#endif
			//open the mpoint /sys...
			fd = (FILE *)fopen(path, "r");
				if(fd==NULL){
				//something is broken...
				fprintf(stderr, "[EE] some problem opening /sys filesystem.\n");
				exit(-1);
				}	
			//get the lines
			lines=(char*)get_line_from_file(fd);
			//close file
			fflush(fd);
			fclose(fd);
			//allocating memory
			temp_sched=(struct md_sched *)malloc(sizeof(struct md_sched));
			temp=(char*)malloc(20);
			//setting VAR
			temp=(char*)parse_line((char*)lines);
			temp_sched->sched=temp;
			temp_sched->dev=(char*)malloc(4);
				snprintf(temp_sched->dev,4,"%s",new->dev);
			temp_sched->next=md_sched_queue;
			//adding at list
			md_sched_queue=temp_sched;
			//select next DEV
			new=new->next;
		}
		free(path);
		//Verbosely Debugging
		#ifdef DEBUG
			//controlling the struct
			temp_sched=md_sched_queue;
			do{			
				fprintf(stderr,"md_sched_queue sched: %s.\n", temp_sched->sched);
				temp_sched=temp_sched->next;
			}while(temp_sched!=NULL);
		#endif
		
		sched=(char*)malloc(4);
		sprintf(sched,"%s",device);
		
		
	}else if(is_network==0){
		//Only 1 DEV
		//build the path
		path=(char*)malloc(32);
		snprintf(path,31,"/sys/block/%s/queue/scheduler",device);

		//open the mpoint /sys
		fd = (FILE *)fopen(path, "r");
			if(fd==NULL){
				//Managing Errors
				fprintf(stderr, "[EE] some problem opening /sys filesystem.\n");
				exit(-1);
			}
	
		//get the lines
		lines=(char*)get_line_from_file(fd);
		//close file
		fflush(fd);
		fclose(fd);
		
		free(path);
		
		//setting VAR
		sched=(char*)malloc(14);
		sched=(char*)parse_line((char*)lines);
	}
		
	
	
	//getting dirty_background_ratio && vfs_cache_pressure value
	path=(char*)malloc(33);
	sprintf(path,"/proc/sys/vm/vfs_cache_pressure");
	//open the mpoint /proc/sys
	fd = fopen(path, "r");
	if(fd==NULL){
		//Managing Error
		fprintf(stderr, "[EE] problems reading vfs_cache_pressure value.\n");
		exit(-1);
	}
	//reading value
	vfs_cache_pressure=(char*)get_line_from_file(fd);
		//Verbosely Debugging
		#ifdef DEBUG
			fprintf(stderr,"Reading vfs_cache_pressure value: %s.\n",vfs_cache_pressure);
		#endif
	//FFlushing and Closing the FD 
	fflush(fd);
	fclose(fd);
	free(path);
	
	path=(char*)malloc(38);
	sprintf(path,"/proc/sys/vm/dirty_background_ratio");
	//open the mpoint /proc/sys
	fd = fopen(path, "r");
	if(fd==NULL){
		//Managing Errors
		fprintf(stderr, "[EE] problems reading dirty_background_ratio value.\n");
		exit(-1);
	}
	//reading values
	dirty_background_ratio=(char*)get_line_from_file(fd);
		//Verbosely Debugging
		#ifdef DEBUG
			fprintf(stderr,"[+] Reading dirty_background_ratio value: %s.\n",dirty_background_ratio);
		#endif
	//FFlushing and Closing the FD 
	fflush(fd);
	fclose(fd);
	free(path);
	free(lines);
}

//This function restore the system environment
void restore_system_env(){	
	char *path;
	FILE *fd;
	struct sched_param parms;
	struct md_sched *temp_sched;
	char *path2;
	
	//Root Check
	if(!I_am_root()){
		fprintf(stderr,"[EE] Can't restore system value.\n");
		return;
	}
	
	//Verbosely Debugging
	#ifdef DEBUG
			fprintf(stderr,"restore_sys_env: before-value_vfs_cache: %s\n",vfs_cache_pressure);
			fprintf(stderr,"restore_sys_env: before-value_dirty_back: %s\n",vfs_cache_pressure);
	#endif
	
	
	if(device==NULL){
		fprintf(stderr, "some problems: have you used get_system_env?\n");
		exit(-1);
	}
	//RAID: Multiple DEVS
	if(is_raid==1 || is_lvm==1){
		temp_sched=md_sched_queue;
		path=(char*)malloc(32);
		while(temp_sched!=NULL){
		
			snprintf(path,31,"/sys/block/%s/queue/scheduler",temp_sched->dev);
			//open the mpoint /sys
			fd = (FILE*)fopen(path, "w");
			if(fd==NULL){
				//Managing Error
				fprintf(stderr, "[EE] some problem opening /sys filesystem: it is mounted?\n");
				exit(-1);
			}
			//write prev IO scheduler
			if(fwrite(temp_sched->sched,strlen(temp_sched->sched),1,fd)<=0){
				//Managing Error
				fprintf(stderr, "[EE] some problem writing scheduler in /sys filesystem.\n");
				fflush(fd);
				fclose(fd);
				exit(-1);
			}
			//FFlushing and Closing the FD 
			fflush(fd);
			fclose(fd);
				//Verbosely Debugging
				#ifdef DEBUG
					fprintf(stderr,"restore_env sched: %s - %s.\n", temp_sched->sched, temp_sched->dev);
					fprintf(stderr,"restore_env path_lenght: %d.\n", strlen(path));
				#endif
			
			//next device
			temp_sched=temp_sched->next;
		}
		free(path);
	}else if(is_network==0){
	
			path=(char*)malloc(32);
			//build path
			snprintf(path,31,"/sys/block/%s/queue/scheduler",device);
	
			//open the mpoint /sys
			fd = (FILE*)fopen(path,"w");
			if(fd==NULL){
			//Managing Errors
					fprintf(stderr, "[EE] some problem opening /sys filesystem: it is mounted?\n");
					//Freeing path
					free(path);
					exit(-1);
				}
			//write prev IO scheduler
			if(fwrite(sched,strlen(sched),1,fd)<=0){
				//something is broken...
				fprintf(stderr, "[EE] some problem writing in /sys filesystem.\n");
				fflush(fd);
				fclose(fd);
				exit(-1);
			}
		//FFlushing and Closing the FD
		fflush(fd);
		fclose(fd);
		free(path);
	}

	//Next Step: setting default process scheduler
	//fprintf(stderr,"SCHEDULER - %d - %d - %d \n\r",SCHED_OTHER,SCHED_FIFO,SCHED_RR);
	parms.sched_priority = 0;
	if(sched_setscheduler(0, SCHED_OTHER, &parms) < 0) {
		fprintf(stderr, "[EE] some problem with process scheduler (SCHED_OTHER).\n");
		exit(-1);
		}
	
	//Next Step setting dirty_background_ratio && vfs_cache_pressure value
		
	path2=(char*)malloc(33);
	sprintf(path2,"/proc/sys/vm/vfs_cache_pressure");
	//open the /proc/sys
	fd = (FILE*)fopen(path2, "w");
	if(fd==NULL){
		//Managing Errors
		fprintf(stderr, "[EE] problems writing vfs_cache_pressure value.\n");
		exit(-1);
	}
	//writing values
	if(fwrite(vfs_cache_pressure,strlen(vfs_cache_pressure),1,fd)<=0){
				//Managing Errors
				fprintf(stderr, "[EE] restore_sys_env: some problem writing vfs_cache_pressure in /sys filesystem.\n");
				//Verbosely Debugging
				#ifdef DEBUG
					fprintf(stderr,"restore_sys_env value_vfs_cache: %s\n",vfs_cache_pressure);
					fprintf(stderr,"restore_sys_env value_dirty_back: %s\n",vfs_cache_pressure);
				#endif
				fclose(fd);
				exit(-1);
			}
	//FFlushing and Closing the FD 
	fflush(fd);
	fclose(fd);
	free(path2);
	
	path2=(char*)malloc(38);
	sprintf(path2,"/proc/sys/vm/dirty_background_ratio");
	//open the /proc/sys...
	fd = (FILE*)fopen(path2, "w");
	if(fd==NULL){
		//Managin Errors
		fprintf(stderr, "[EE] problems writing dirty_background_ratio value.\n");
		exit(-1);
	}
	//reading values
	//writing values
	if(fwrite(dirty_background_ratio,strlen(dirty_background_ratio),1,fd)<=0){
				//something is broken...
				fprintf(stderr, "[EE] restore_sys_env: some problem writing dirty_background_ratio in /sys filesystem.\n");
				fclose(fd);
				exit(-1);
			}
	//FFlushing and Closing the FD 
	fflush(fd);
	fclose(fd);
	free(path2);
	
	fprintf(stderr, "[I] System default values restored!\n");
}

//This Function changes the IO scheduler in a mode passed by a value in INPUT
//INPUT: (char *) with value. The value means
// 1 - anticipatory, 
// 2 - cfq,
// 3 - noop, 
// 4 - deadline
//REMEMBER: in case of errors, it blocks program execution.
//REMEMBER: use get_system_env before the use of this function
void set_IO_sched(char *set_value){
	
	int fd, option=0;
	char *path, *anticipat = "anticipatory", *nop = "noop", *cfq = "cfq", *dead = "deadline";
	//struct sched_param parms;
	struct md_sched *temp_sched;
	
	//ROOT checks
	if(!I_am_root()){
		fprintf(stderr,"Can't set IO scheduler.\n");
		return;
	}
	
	path=(char*)malloc(32);
	//create strings
	//Anticipatory
	if(strcmp(anticipat,set_value)==0){
		option=1;
	}
	 //CFQ
	if(strcmp(cfq,set_value)==0){
		option=2;
	}
	//Noop
	if(strcmp(nop,set_value)==0){
		option=3; 
	}
	//Deadline
	if(strcmp(dead,set_value)==0){
		option=4; 
	}
	
	if(device==NULL){
		//device var is NOT initialized. Init it!
		//Managing errors
		fprintf(stderr, "[EE] you MUST use get_dev_by_mnt function before using set_sched.\n");
		exit(-1);
	}
	
	if(is_raid==1){
		temp_sched=md_sched_queue;
		do{
			sprintf(path,"/sys/block/%s/queue/scheduler",temp_sched->dev);
			//open the mpoint /sys
			fd = open(path, O_RDWR);
			if(fd==0){
				//Managing Errors
				fprintf(stderr, "[EE] set_IO_sched - some problem opening /sys filesystem: it is mounted?\n");
				exit(-1);
			}
			//writing message
			fprintf(stderr, "On dev %s", temp_sched->dev);
			//write prev IO scheduler
			switch(option){
				case 1:
					if(write(fd,"anticipatory",12)<=0){
					//Managing Errors
						fprintf(stderr, "some problem writing in /sys filesystem.\n");
						close(fd);
						exit(-1);
					}
					fprintf(stderr, "[anticipatory] scheduler selected.\n");
					break;
				case 2:
					if(write(fd,"cfq",3)<=0){
						//Managing Errors
						fprintf(stderr, "some problem writing in /sys filesystem.\n");
						close(fd);
						exit(-1);
						}
					fprintf(stderr, "[cfq] scheduler selected.\n");
					break;
				case 3:
					if(write(fd,"noop",4)<=0){
							//Managing Errors
						fprintf(stderr, "some problem writing in /sys filesystem.\n");
						close(fd);
						exit(-1);
						}
					fprintf(stderr, "[noop] scheduler selected.\n");
					break;
				case 4:
					if(write(fd,"deadline",8)<=0){
							//Managing Errors
						fprintf(stderr, "some problem writing in /sys filesystem.\n");
						close(fd);
						exit(-1);
					}
					fprintf(stderr, "[deadline] scheduler selected.\n");
					break;
				default:
					fprintf(stderr, "[EE] linux-IO-scheduler: wrong option. Select a valid scheduler.\n");
					close(fd);
					exit(-1);
					break;
				}
				//Verbosely Debugging
				#ifdef DEBUG
					fprintf(stderr,"[EE] set_IO_sched sched: %s - %s.\n", temp_sched->sched, temp_sched->dev);
				#endif
			//close file
			close(fd);
			temp_sched=temp_sched->next;
		}while(temp_sched!=NULL);
	}else if(is_network==0){
	
		sprintf(path,"/sys/block/%s/queue/scheduler",device);
		//open the mpoint /sys
		fd = open(path, O_RDWR);
			if(fd<0){
					//Managing Errors
				fprintf(stderr, "some problem opening /sys filesystem: it is mounted or you have this scheduler compiled in kernel?\n");
				exit(-1);
			}
	
		//writing message
			fprintf(stderr, "[I] On dev %s - ", device);
		//setting up the scheduler chosen
		switch(option){
			case 1:
				if(write(fd,"anticipatory",12)<=0){
					//Managing Errors
				fprintf(stderr, "some problem writing in /sys filesystem.\n");
				close(fd);
				exit(-1);
				}
			fprintf(stderr, "[anticipatory] scheduler selected.\n");
			break;
		case 2:
			if(write(fd,"cfq",3)<=0){
					//Managing Errors
				fprintf(stderr, "some problem writing in /sys filesystem.\n");
				close(fd);
				exit(-1);
				}
			fprintf(stderr, "[cfq] scheduler selected.\n");
			break;
		case 3:
			if(write(fd,"noop",4)<=0){
					//Managing Errors
				fprintf(stderr, "some problem writing in /sys filesystem.\n");
				close(fd);
				exit(-1);
				}
			fprintf(stderr, "[noop] scheduler selected.\n");
			break;
		case 4:
			if(write(fd,"deadline",8)<=0){
					//Managing Errors
				fprintf(stderr, "some problem writing in /sys filesystem.\n");
				close(fd);
				exit(-1);
				}
			fprintf(stderr, "[deadline] scheduler selected.\n");
			break;
		default:
			fprintf(stderr, "wrong option.\n");
			close(fd);
			exit(-1);
			break;
		}
	
		//SYNC and CLOSE the FD
		sync();
		close(fd);
		free(path);
	}else{
		//In case of Network File System this message is printed out and no scheduler is setted
		fprintf(stderr, "is a network fs: I can't change the IO scheduler.\n");
	}
}

//This function sets the vfs_cache_pressure
void set_vfs_cache_pressure(int pres){
	
	char *path, *value;
	int fd;
	
	//Root Checks
	if(!I_am_root()){
		fprintf(stderr,"[EE] Can't set vfs_cache_pressure.\n");
		return;
	}
	
	if(pres>1000 || pres<0){
		fprintf(stderr, "[EE] vfs_cache_pressure value is not correct.\n");
		exit(-1);
	}
	path=(char*)malloc(35);
	sprintf(path,"/proc/sys/vm/vfs_cache_pressure");
	//Open the mpoint /proc/sys
	fd = open(path, O_RDWR);
	if(fd<0){
		//Fault in setting VFS cache pressure
		fprintf(stderr, "[EE] problems with vfs_cache_pressure.\n");
		exit(-1);
	}
	value=(char*)malloc(5);
	snprintf(value,4,"%d",pres);
	//write value
	if(write(fd,value,8)<=0){
		//Fault in setting VFS cache pressure
		fprintf(stderr, "[EE] error setting vfs_cache_pressure.\n");
		close(fd);
		exit(-1);
		}
		//Closing FD
	close(fd);
	free(path);
	free(value);
	fprintf(stderr, "[I] vfs_cache_pressure value setted.\n");
	//exiting
}

//Thid function sets the dirty_background_ratio
void set_dirty_background_ratio(int pres){	
	char *path, *value;
	int fd;
	
	//Root Checks
	if(!I_am_root()){
		fprintf(stderr,"[EE] Can't set dirty_background_ratio.\n");
		return;
	}
	
	if(pres>100 || pres<0){
		fprintf(stderr, "[EE] dirty_background_ratio value is not correct.\n");
		exit(-1);
	}
	path=(char*)malloc(35);
	sprintf(path,"/proc/sys/vm/dirty_background_ratio.\n");
	//open the mpoint /proc/sys
	fd = open(path, O_RDWR);
	if(fd<0){
		//Fault with dirty_background_ratio management
		fprintf(stderr, "[EE] problems with dirty_background_ratio.\n");
		exit(-1);
	}
	value=(char*)malloc(5);
	snprintf(value,4,"%d",pres);
	//write values
	if(write(fd,value,8)<=0){
		//Fault with dirty_background_ratio management
		fprintf(stderr, "[EE] error setting dirty_background_ratio.\n");
		close(fd);
		exit(-1);
		}
	//Closind FD and terminate normally
	close(fd);
	free(path);
	free(value);
	fprintf(stderr, "[I] dirty_background_ratio value setted.\n");
}

//This Function Locks the memory
// Cast -1 on error
int lock_memory(void)
{
	//locking present and future memory
	//Root Checks
	if(!I_am_root()){
		fprintf(stderr,"Can't lock memory.\n");
		return -1;
	}
	//MEMLOCK current
	if(mlockall(MCL_CURRENT)) 
		{
		fprintf(stderr,"[EE] error with mlockall / CURRENT.\n");
		return -1; 
		}	
	//MEMLOCK future
	if(mlockall(MCL_FUTURE)) 
		{	
		fprintf(stderr,"[EE] error with mlockall / FUTURE.\n"); 
		return -1; 
		}		
	return (int)1;
}

//This Function Unlocks the memory
int unlock_memory(void)
{
	//Unlocking all memory
		//Root Checks
	if(!I_am_root()){
		fprintf(stderr,"[EE] Can't unlock memory.\n");
		return -1;
	}
	if(munlockall()) 
		{			
		fprintf(stderr,"[EE] error with munlockall.\n"); 
		return -1;
		}	
	return (int)1;
}

#endif
