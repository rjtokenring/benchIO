// [benchIO]
// 
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File thread_warmup.c 
// Last Revision:   23th June, 2005
//
// thread_warmup.c :
// 1) warmup thread (using all functions that will be used during benchmark)

#include "header/main.h"

void warm_up_Icache(struct thread_params* tp)
{
	FILE *fp=NULL;
	int point=0;
	char filepath[1000] = "", res;
	mode_t perm = 00600;
	struct time_elapsed test_time;
	
	strcat(filepath, tp->mount_point);
	strcat(filepath, "warm_up_file");
	
	get_timing(tp->gettimes, &test_time);

	if(tp->posix_rw == 0)
	{
		//Warming up fopen, fread,fwrite and so on (ANSI C)
		fp=(FILE*)fopen(filepath,"wr");
		fchmod(fileno(fp), perm);
		fwrite("a",sizeof(char),1,fp);
		fread(&res,sizeof(char),1,fp);
		fflush(fp);
		fclose(fp);
	}
	else
	{
		//Warming up open, read,write and so on (POSIX)
		point=open(filepath, O_CREAT | O_TRUNC | O_RDWR);
		fchmod(point, perm);
		write(point,"a",sizeof(char));
		read(point,&res,sizeof(char));
		fsync(point);
		close(point);
	}
	
	unlink(filepath);
}
 
