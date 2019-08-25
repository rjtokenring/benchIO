// [benchIO]
//
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File thread.c
// Last Revision:   23th June, 2005
//
// thread.c :
// 1) thread-level warmup
// 2) initializing random number generator
// 3) pseudo-random block generation, ANSI or POSIX file opening, writing, flushing and closing, in sequential or random access mode, with time measurements
// 4) ANSI or POSIX file opening, reading and closing, in sequential or random access mode, with time measurements
// 5) test file unlinking
// 6) results displaying
// 7) thread semaphore waiting

#include "header/main.h"

//Allocate memory aligned to 512 bytes with main memory (for O_DIRECT)
char* aligned_malloc(size_t size, char** not_aligned_ptr)
{
	*not_aligned_ptr = (char*) malloc(size + 512);
	return (char*)((unsigned long int) *not_aligned_ptr + (512 - ((unsigned long int) *not_aligned_ptr % 512)));
}

//Free aligned memory
void not_aligned_free(char** not_aligned_ptr)
{
	free(*not_aligned_ptr);
}

//Runs I/O operations on files and takes times
int io_operations(struct thread_params* tp)
{
	//Handler for posix I/O
	off_t handler = 0;
	//Pointer for ANSI I/O
	FILE* fp = NULL;
	//Rights of the created file
	mode_t perm = 00600;
	//Flags used for writing and reading
	int flags = 0;
	//Strings used for filename and filepath
	char filename[25] = "";
	char filepath[1000] = "";
	//Generic variable
	unsigned long int i = 0;
	//Buffer for the block to write to and read from file
	char* buffer = NULL;
	//Length of the buffer
	unsigned long int buflen = 0;
	//Numer of blocks in the file
	unsigned long nblocks = 0;
	//Returning value of the called routine
	int result = 0;
	//Variable for sequential or random file acess
	int random = 0;
	//Variable for value returned by this routine
	int returned = 0;
	//Structs for time measurements
	struct time_elapsed write_time_measure_OLD;
	struct time_elapsed write_time_measure_NEW;
	struct time_elapsed write_time_measure_DELTA;
	struct time_elapsed read_time_measure_OLD;
	struct time_elapsed read_time_measure_NEW;
	struct time_elapsed read_time_measure_DELTA;
	//Pointer for aligned malloc
	char* not_aligned_ptr = NULL;
	//Struct for test results
	struct test_result results;
	
	unsigned long int j = 0;
	int tmp = 0;
	float bl = 0;
	float rm = 0;

	//Sets test's filename
	if(tp->mode == 1) { sprintf(filename, "/%s_1", RESFILE); }
	if(tp->mode == 2) { sprintf(filename, "/%s_2", RESFILE); }
	if(tp->mode == 3) { sprintf(filename, "/%s_3_%d", RESFILE, tp->thread); }
	if(tp->mode == 4) { sprintf(filename, "/%s_4", RESFILE); }
	if(tp->mode == 5) { sprintf(filename, "/%s_5_%d", RESFILE, getpid()); }
	if(tp->mode == 6) { sprintf(filename, "/%s_6_%d", RESFILE, getpid()); }
	if(tp->mode == 7) { sprintf(filename, "/%s_7_%d_%d", RESFILE, getpid(), tp->thread); }
	strcat(filepath, tp->mount_point);
	strcat(filepath, filename);

	//Verbosely Debug
	#ifdef DEBUG
	fprintf(stderr, "Thread #%d: I/O operations on file %s:\n", tp->thread, filepath);
	#endif

	//Inits RTC clock
	#ifdef LINUX
	if(tp->gettimes == 2) system_rtc_set(RTC_FREQUENCY);
	#endif
	
	//If not WINDOWS, warmup to store routines in istruction cache
	#ifndef WINDOWS
	//warm up istruction cache...
	warm_up_Icache(tp);
	#endif

	//Initalizes ramdom number generator
	srand((unsigned) time(NULL));

	//Calculates buffer length
	buflen = tp->blocksize * 1024;
	bl = (float) buflen;
	rm = (float) RAND_MAX;
	
	//Calculates number of blocks of this file
	nblocks = tp->filesize / tp->blocksize;
	
	//Allocates memory for buffer
	if(tp->o_direct == 0)
		buffer = (char*) malloc(buflen * sizeof(char));
	else
		buffer = aligned_malloc(buflen * sizeof(char), &not_aligned_ptr);

	//Cycle for sequential and random access on test file
	for(random = 0; random <= 1; random++)
	{

// Write Code Portion
		
		//If opening time is included in measurements, takes time here
		if(tp->include_flush == 1){
			//Gets starting time
			get_timing(tp->gettimes, &write_time_measure_OLD);
		}
		
		//If posix test selected, runs posix, else ansi
		if(tp->posix_rw == 0)
		{
			//Opens file
			fp = (FILE*) fopen(filepath, "w");
			if(fp == NULL) { fprintf(stderr, "Thread #%d: Error opening file to write to\nFunction fopen(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); return -1; }

			//Verbosely Debug
			#ifdef DEBUG
			fprintf(stderr, "Thread #%d: File %s opened in buffered I/O mode for %s writing\n", tp->thread, filepath, random == 0 ? "sequential" : "random");
			#endif

			//Changes rights
			result = fchmod(fileno(fp), perm);
			if(result == -1) { fprintf(stderr, "Thread #%d: Error changing file's permissions\nFunction chmod(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); returned = -1; }
			
			//Puts file cursor at the beginning of the file
			rewind(fp);
		}
		else
		{
			//Sets file opening flags
			flags = O_CREAT | O_TRUNC | O_WRONLY;
			if(tp->o_sync == 1) flags |= O_SYNC;
			#ifdef LINUX
			if(tp->o_direct == 1) flags |= O_DIRECT;
			#endif
			
			//Opens file
			handler = open(filepath, flags);
			if(handler == -1) { fprintf(stderr, "Thread #%d: Error opening file to write to\nFunction open(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); return -1; }

			//Verbosely Debug
			#ifdef DEBUG
			fprintf(stderr, "Thread #%d: File %s opened in POSIX mode for %s writing\n", tp->thread, filepath, random == 0 ? "sequential" : "random");
			#endif

			//Changes rights
			result = fchmod(handler, perm);
			if(result == -1) { fprintf(stderr, "Thread #%d: Error changing file's permissions\nFunction chmod(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); returned = -1; }
			
			//Puts file cursor at the beginning of the file
			if(lseek(handler, 0, SEEK_SET) == (off_t)-1) { fprintf(stderr, "Thread #%d: Error repositioning file's cursor\nFunction lseek(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); returned = -1; }
		}
		
		//If opening time is not included in measurements, takes time here
		if(tp->include_flush == 0){
			//Gets starting time
			get_timing(tp->gettimes, &write_time_measure_OLD);
		}

		//If no errors
		if(returned != -1)
		{
			//If no errors, runs write test
			if(buffer == NULL) { fprintf(stderr, "Thread #%d: Error allocating buffer for block\nFunction malloc(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); returned = -1; }
			else
			{
				//Verbosely Debug
				#ifdef DEBUG
				fprintf(stderr, "Thread #%d: Start writing on file %s\n", tp->thread, filepath);
				#endif

				//Verbosely Debug
				#ifdef DEBUG
				fprintf(stderr, "--- Useless Times ---\n");
				#endif
				
				//Cycle to write each block of the file
				for(i = 0; i < tp->filesize; i += tp->blocksize)
				{
					//Verbosely Debug
					#ifdef DEBUG
					fprintf(stderr, "Thread #%d: Filling up buffer with random data... ", tp->thread);
					#endif

					//Cycle to fill buffer with random chars (not all buffer, only some randomly selected chars)
					for(j = 0; j < tp->blocksize; j++)
					{
						tmp = rand();
						buffer[(unsigned long int)(bl * ((float) tmp / rm))] = (char) tmp;
					}

					//Verbosely Debug
					#ifdef DEBUG
					fprintf(stderr, "Done\n");
					#endif

					//Ansi or posix writing
					if(tp->posix_rw == 0)
					{
						//If random file access, randomly selects a block
						if(random == 1) if(fseek(fp, buflen * (unsigned long int)((rand() / RAND_MAX) * (nblocks - 1)), SEEK_SET) == -1) { fprintf(stderr, "Thread #%d: Error seeking in file\nFunction fseek()\n", tp->thread); returned = -1; break; }
						
						//Writes block on file
						fwrite(buffer, buflen, 1, fp);
						if(ferror(fp) != 0) { fprintf(stderr, "Thread #%d: Error writing block %lu\nFunction fwrite()\n", tp->thread, i / tp->blocksize); returned = -1; break; }
					}
					else
					{
						//If random file access, randomly selects a block
						if(random == 1) if(lseek(handler, buflen * (unsigned long int)((rand() / RAND_MAX) * (nblocks - 1)), SEEK_SET) == (off_t)-1) { fprintf(stderr, "Thread #%d: Error seeking in file\nFunction lseek()\n", tp->thread); returned = -1; break; }
						
						//Writes block on file
						result = write(handler, buffer, buflen);
						if(result == 0) { fprintf(stderr, "Thread #%d: Written zero bytes of block %lu\nFunction write()\n", tp->thread, i / tp->blocksize); returned = -1; break; }
						if(result == -1) { fprintf(stderr, "Thread #%d: Error writing block %lu\nFunction write(), errno = %d: %s\n", tp->thread, i / tp->blocksize, errno, strerror(errno)); returned = -1; break; }
					}
				}
				
				//Verbosely Debug
				#ifdef DEBUG
				fprintf(stderr, "Thread #%d: Stop writing on file %s\n", tp->thread, filepath);
				#endif
			}
		}
	
		//If flushing is not included in time measurements, takes time here
		if(tp->include_flush == 0) {
			//Gets stopping time
			get_timing(tp->gettimes, &write_time_measure_NEW);
		}
		
		//Ansi or posix access
		if(tp->posix_rw == 0)
		{
			//Flushes buffers
			result = fflush(fp);
			if(result == -1) { fprintf(stderr, "Thread #%d: Error flushing buffers\nFunction fflush(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); returned = -1; }

			//Verbosely Debug
			#ifdef DEBUG
			fprintf(stderr, "Thread #%d: Flushing buffers for file %s\n", tp->thread, filepath);
			#endif

			//Closes file
			result = fclose(fp);
			if(result != 0) { fprintf(stderr, "Thread #%d: Error closing file\nFunction fclose(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); returned = -1; }

			//Verbosely Debug
			#ifdef DEBUG
			fprintf(stderr, "Thread #%d: File %s closed\n", tp->thread, filepath);
			#endif
		}
		else
		{
			//Closes file
			result = close(handler);
			if(result != 0) { fprintf(stderr, "Thread #%d: Error closing file\nFunction close(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); returned = -1; }

			//Verbosely Debug
			#ifdef DEBUG
			fprintf(stderr, "Thread #%d: File %s closed\n", tp->thread, filepath);
			#endif
		}
	
		//If error returns
		if(returned == -1) return returned;

		//Verbosely Debug
		#ifdef DEBUG
		fprintf(stderr, "--- %s Write Times ---\nFile: %lu K, Block: %lu K\n", random == 0 ? "Sequential" : "Random", tp->filesize, tp->blocksize);
		#endif
		
		//If flushing is included in time measurements, takes time here
		if(tp->include_flush == 1){
			//Gets stopping time
			get_timing(tp->gettimes, &write_time_measure_NEW);
		}
		
		//Calculates difference of starting time ad stopping time
		get_delta(&write_time_measure_OLD, &write_time_measure_NEW, &write_time_measure_DELTA,tp->gettimes);

		//Store results in results' struct
		if(random == 0)
		{
			results.swt = write_time_measure_DELTA.total;
			results.swu = write_time_measure_DELTA.user_time;
			results.sws = write_time_measure_DELTA.system_time;
			results.swc = write_time_measure_DELTA.cpu_usage;
		}
		else
		{
			results.rwt = write_time_measure_DELTA.total;
			results.rwu = write_time_measure_DELTA.user_time;
			results.rws = write_time_measure_DELTA.system_time;
			results.rwc = write_time_measure_DELTA.cpu_usage;
		}

// Read Code Portion

		//If opening time is included in measurements, takes time here
		if(tp->include_flush == 1){
			//Gets starting time
			get_timing(tp->gettimes, &read_time_measure_OLD);
		}
	
		//If posix test selected, run posix, else ansi
		if(tp->posix_rw == 0)
		{
			//Opens file
			fp = (FILE*) fopen(filepath, "r");
			if(fp == NULL) { fprintf(stderr, "Thread #%d: Error opening file to read\nFunction fopen(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); return -1; }

			//Verbosely Debug
			#ifdef DEBUG
			fprintf(stderr, "Thread #%d: File %s opened in buffered I/O mode for %s reading\n", tp->thread, filepath, random == 0 ? "sequential" : "random");
			#endif
		}
		else
		{
			//Sets flags
			flags = O_RDONLY;
			
			//Opens file
			handler = open(filepath, flags);
			if(handler == -1) { fprintf(stderr, "Thread #%d: Error opening file to read\nFunction open(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); return -1; }

			//Verbosely Debug
			#ifdef DEBUG
			fprintf(stderr, "Thread #%d: File %s opened in POSIX mode for %s reading\n", tp->thread, filepath, random == 0 ? "sequential" : "random");
			#endif
		}

		//If opening time is not included in measurements, takes time here
		if(tp->include_flush == 0){
			//Gets starting time
			get_timing(tp->gettimes, &read_time_measure_OLD);
		}

		//If no errors, runs read test
		if(buffer == NULL) { fprintf(stderr, "Thread #%d: Error allocating buffer for block\nFunction malloc(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); returned = -1; }
		else
		{
			//Verbosely Debug
			#ifdef DEBUG
			fprintf(stderr, "Thread #%d: Start reading file %s\n", tp->thread, filepath);
			#endif

			//Verbosely Debug
			#ifdef DEBUG
			fprintf(stderr, "--- Useless Times ---\n");
			#endif
		
			//Cycle to read each block of the file
			for(i = 0; result == 0 && i < tp->filesize; i += tp->blocksize)
			{
				//Ansi or posix
				if(tp->posix_rw == 0)
				{
					//If random access, randomly selects a block
					if(random == 1) if(fseek(fp, buflen * (unsigned long int)((rand() / RAND_MAX) * (nblocks - 1)), SEEK_SET) == -1) { fprintf(stderr, "Thread #%d: Error seeking in file\nFunction fseek()\n", tp->thread); returned = -1; break; }
					
					//Reads a block from file
					fread(buffer, buflen, 1, fp);
					if(ferror(fp) != 0) { fprintf(stderr, "Thread #%d: Error reading block %lu\nFunction fread()\n", tp->thread, i / tp->blocksize); returned = -1; break; }
					
					//Tests if end of file is reached
					result = feof(fp);
				}
				else
				{
					unsigned long int r = 0;
					unsigned long int br = 0;
	
					//If random access, randomly selects a block
					if(random == 1) if(lseek(handler, buflen * (unsigned long int)((rand() / RAND_MAX) * (nblocks - 1)), SEEK_SET) == (off_t)-1) { fprintf(stderr, "Thread #%d: Error seeking in file\nFunction lseek()\n", tp->thread); returned = -1; break; }
					
					//Read all bytes of the block
					while(br < buflen)
					{
						r = (unsigned long int) read(handler, buffer + br, buflen - br);
						if(r == 0) break;
						br += r;
					}
					if(r == -1) { fprintf(stderr, "Thread #%d: Error reading block %lu\nFunction read(), errno = %d: %s\n", tp->thread, i / tp->blocksize, errno, strerror(errno)); returned = -1; break; }
				}
			}
			
			//Verbosely Debug
			#ifdef DEBUG
			fprintf(stderr, "Thread #%d: Stop reading from file %s\n", tp->thread, filepath);
			#endif
		}
		
		//If closing time is not included in measurements, takes time here
		if(tp->include_flush == 0){ 
			get_timing(tp->gettimes, &read_time_measure_NEW);
		}
		
		//Ansi or posix
		if(tp->posix_rw == 0)
		{
			//Closes file
			result = fclose(fp);
			if(result != 0) { fprintf(stderr, "Thread #%d: Error closing file\nFunction fclose(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); returned = -1; }

			//Verbosely Debug
			#ifdef DEBUG
			fprintf(stderr, "Thread #%d: File %s closed\n", tp->thread, filepath);
			#endif
		}
		else
		{
			result = close(handler);
			if(result != 0) { fprintf(stderr, "Thread #%d: Error closing file\nFunction close(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); returned = -1; }

			//Verbosely Debug
			#ifdef DEBUG
			fprintf(stderr, "Thread #%d: File %s closed\n", tp->thread, filepath);
			#endif
		}

		//Verbosely Debug
		#ifdef DEBUG
		fprintf(stderr, "--- %s Read Times ---\nFile: %lu K, Block: %lu K\n", random == 0 ? "Sequential" : "Random", tp->filesize, tp->blocksize);
		#endif

		//If closing time is included in measurements, takes time here
		if(tp->include_flush == 1){ 
			get_timing(tp->gettimes, &read_time_measure_NEW);
		}

		//Claculates difference of starting and stopping times
		get_delta(&read_time_measure_OLD, &read_time_measure_NEW, &read_time_measure_DELTA,tp->gettimes);

		//Stores results in struct
		if(random == 0)
		{
			results.srt = read_time_measure_DELTA.total;
			results.sru = read_time_measure_DELTA.user_time;
			results.srs = read_time_measure_DELTA.system_time;
			results.src = read_time_measure_DELTA.cpu_usage;
		}
		else
		{
			results.rrt = read_time_measure_DELTA.total;
			results.rru = read_time_measure_DELTA.user_time;
			results.rrs = read_time_measure_DELTA.system_time;
			results.rrc = read_time_measure_DELTA.cpu_usage;
		}
	}

	//Shuts down RTC
	#ifdef LINUX
	if(tp->gettimes == 2) system_rtc_down();
	#endif	

	//Frees memory allocated for buffer
	if(tp->o_direct == 0)
		free(buffer);
	else
		not_aligned_free(&not_aligned_ptr);

// Unlink Code Portion
	
	//If mode is 1, 3, 5 or 7
	if(tp->mode == 1 || tp->mode == 3 || tp->mode == 5 || tp->mode == 7)
	{
		//Unlinks file
		result = unlink(filepath);
		if(result != 0) { fprintf(stderr, "Thread #%d: Error unlinking (deleting) file\nFunction unlink(), errno = %d: %s\n", tp->thread, errno, strerror(errno)); returned = -1; }

		//Verbosely Debug
		#ifdef DEBUG
		fprintf(stderr, "Thread #%d: File %s unlinked\n", tp->thread, filepath);
		#endif
	}

	#ifdef DEBUG
	//Prints results on screen
	fprintf(stderr,
		"/^^^^^^^^^^^\n"
		"| File size: %lu K, Block size: %lu K\n"
		"| Sequential Write: total        time: %f s\n"
		"|                   throughput       : %f Kb/s\n"
		"|                   user         time: %f s\n"
		"|                   system       time: %f s\n"
		"|                   cpu usage        : %f\n"
		"| Sequential Read:  total        time: %f s\n"
		"|                   throughput       : %f Kb/s\n"
		"|                   user         time: %f s\n"
		"|                   system       time: %f s\n"
		"|                   cpu usage        : %f\n"
		"| Random Write:     total        time: %f s\n"
		"|                   throughput       : %f Kb/s\n"
		"|                   user         time: %f s\n"
		"|                   system       time: %f s\n"
		"|                   cpu usage        : %f\n"
		"| Random Read:      total        time: %f s\n"
		"|                   throughput       : %f Kb/s\n"
		"|                   user         time: %f s\n"
		"|                   system       time: %f s\n"
		"|                   cpu usage        : %f\n"
		"\\___________\n", 
		tp->filesize,
		tp->blocksize,
		results.swt,
		tp->filesize / results.swt,
		results.swu,
		results.sws,
		results.swc,
		results.srt,
		tp->filesize / results.srt,
		results.sru,
		results.srs,
		results.src,
		results.rwt,
		tp->filesize / results.rwt,
		results.rwu,
		results.rws,
		results.rwc,
		results.rrt,
		tp->filesize / results.rrt,
		results.rru,
		results.rrs,
		results.rrc
	);	
	#endif

	//If mode is 2 or 3 (multi thread)
	if((tp->mode == 2 || tp->mode == 3) && tp->num_thr_pro > 1)
	{
		//Increments semaphore
		increment_thread_flag(tp);
		//Waits on condition

		//Verbosely Debug
		#ifdef DEBUG
		fprintf(stderr,"Thread #%d: first increment!!! %d - num_th %d.\n", tp->thread, *tp->cond_var, tp->num_thr_pro);
		#endif	

		if(pthread_mutex_lock(tp->thread_flag_mutex) < 0)
			fprintf(stderr, "Thread #%d: mutex error 1.\n", tp->thread);

		//Cycle to wait condition variable
		while(*tp->cond_var < (tp->num_thr_pro - 3 * (tp->num_thr_pro / 5)))
			pthread_cond_wait(tp->thread_flag_cv, tp->thread_flag_mutex);

		//Unlocks mutex
		pthread_mutex_unlock(tp->thread_flag_mutex);

		//Verbosely Debug
		#ifdef DEBUG
		fprintf(stderr, "Thread #%d: ---------- I'm unlocked!\n", tp->thread);
		#endif	
		
		//Writes results in file
		if(tp->warmup == 0)
			write_result_on_file(tp, &results);
	}
	//If mode is 4 or 5 (multi process)
	else if((tp->mode == 4 || tp->mode == 5) && tp->num_thr_pro > 1)
	{
		#ifdef WINDOWS
		//noop - cygwin doesn't support this kind of semaphores.
		#else
		//Decrements semaphore
		binary_semaphore_wait(tp->sem_ident);
		
		//Waits until value of semaphore is 0
		binary_semaphore_wait_zero(tp->sem_ident);
		
		//Writes results in file
		if(tp->warmup == 0)
			write_result_on_file(tp, &results);
		#endif

		//Verbosely Debug
		#ifdef DEBUG
		fprintf(stderr, "Thread #%d of process %d: finished.\n", tp->thread, tp->process);
		#endif	
	}
	//If mode is 6 or 7 (multi process, multi thread)
	else if(tp->mode == 6 || tp->mode == 7)
	{
		//Increments semaphore
		increment_thread_flag(tp);

		//Verbosely Debug
		#ifdef DEBUG
		fprintf(stderr, "Thread #%d: first increment!!! %d - num_th %d\n", tp->thread, *tp->cond_var, tp->num_thr_pro);
		#endif	

		if(pthread_mutex_lock(tp->thread_flag_mutex) < 0)
			fprintf(stderr, "Thread #%d: mutex error 1", tp->thread);

		//Cycle to wait condition variable
		while(*tp->cond_var < (tp->num_thr_pro - 3 * (tp->num_thr_pro / 5)))
			pthread_cond_wait(tp->thread_flag_cv, tp->thread_flag_mutex);

		//Unlocks mutex
		pthread_mutex_unlock(tp->thread_flag_mutex);

		//Writes results in file
		if(tp->warmup == 0)
			write_result_on_file(tp, &results);
	}
	//If mode is 1
	else
	{
		//Writes results in file
		if(tp->warmup == 0)
			write_result_on_file(tp, &results);
	}

	//Returns value
	return returned;
}

//Thread main function
void* thread_function(void* params)
{
	//Value returned by thread
	int returned = 0;
	//Struct for thread parameters
	struct thread_params* tp = (struct thread_params*) params;

	//Verbosely Debug
	#ifdef DEBUG
	fprintf(stderr, "Thread #%d: Begin\n", tp->thread);
	#endif

	//Runs tests
	returned = io_operations(tp);

	//Verbosely Debug
	#ifdef DEBUG
	fprintf(stderr, "Thread #%d: End\n", tp->thread);
	#endif

	//Returns value
	return (void*) returned;
}



