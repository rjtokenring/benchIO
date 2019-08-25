// [benchIO]
//
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File process.c
// Last Revision:   23th June, 2005
//
// process.c :
// 1) nested cycles to run all tests
// 2) children forking for multi-process mode
// 3) threads launching
// 4) threads waiting
// 5) children waiting
// 6) call to csv-assembling function

#include "header/main.h"

//Default Variables
//Number of processes 1
int processes = 1;
//ID process 0
int process = 0;
//Number of thread 1
int threads = 1;
//ID thread 0
int thread = 0;

//Variables to manage filesize and blocksize
unsigned long int filesize = 0;
unsigned long int filesize_min = 0;
unsigned long int filesize_max = 0;
unsigned long int blocksize = 0;
unsigned long int blocksize_min = 0;
unsigned long int blocksize_max = 0;

//Vars to manage processes
pid_t pid = 0;
pid_t pids[1000];

//Vars to manage threads
pthread_t tid = 0;
pthread_t tids[1000];

//Structs to manage thread parameters
struct thread_params* thrpar[1000];
struct thread_params* thr_par = NULL;

//Chid process termination status
int status = 0;

//Vars for thread and process group
int process_group = 0;
int thread_group = 0;

//semaphore unique ID
static int sem_ident;

//This functions cleans the memory allocated by the child
void clean_child()
{
	for(thread = 0; thread < threads; thread++)
		if(thrpar[thread] != NULL)
			free(thrpar[thread]);

	#ifdef DEBUG
	fprintf(stderr, "Child #%d: Allocated memory for structs now freed\n", process);
	#endif
}

//This function detaches threads
void detach_threads()
{
	for(thread = 0; thread < threads; thread++)
	{
		pthread_detach(tids[thread]);
	}

	#ifdef DEBUG
	fprintf(stderr, "Child #%d: Remaining threads detached\n", process);
	#endif
}

//The BenchIO Engine Main Function here
void benchmark(struct options* opts)
{
// Startup Operations Section

	filesize_min = FILESIZE_MIN;
	filesize_max = opts->maxfsize;
	blocksize_min = BLOCKSIZE_MIN;
	blocksize_max = BLOCKSIZE_MAX;

/*
	//Writing benchmark option on a binary file
	if(opts->warmup == 0)
		write_options_info_on_file(opts);
*/

	if(opts->mode == 1){ processes = 1; threads = 1; }
	if(opts->mode == 2){ processes = 1; threads = opts->number; blocksize_min = BLOCKSIZE_MAX_MULTI; blocksize_max = BLOCKSIZE_MAX_MULTI; }
	if(opts->mode == 3){ processes = 1; threads = opts->number; filesize_max = FILESIZE_MAX_MULTI; blocksize_min = BLOCKSIZE_MAX_MULTI; blocksize_max = BLOCKSIZE_MAX_MULTI; }
	if(opts->mode == 4){ processes = opts->number; threads = 1; blocksize_min = BLOCKSIZE_MAX_MULTI; blocksize_max = BLOCKSIZE_MAX_MULTI; }
	if(opts->mode == 5){ processes = opts->number; threads = 1; filesize_max = FILESIZE_MAX_MULTI; blocksize_min = BLOCKSIZE_MAX_MULTI; blocksize_max = BLOCKSIZE_MAX_MULTI; }
	if(opts->mode == 6){ processes = opts->number; threads = opts->number; filesize_max = FILESIZE_MAX_MULTI; blocksize_min = BLOCKSIZE_MAX_MULTI; blocksize_max = BLOCKSIZE_MAX_MULTI; }
	if(opts->mode == 7){ processes = opts->number; threads = opts->number; filesize_max = FILESIZE_MAX_MULTI; blocksize_min = BLOCKSIZE_MAX_MULTI; blocksize_max = BLOCKSIZE_MAX_MULTI; }

	if(opts->warmup == 1)
	{
		//Environment Warmup Options
		filesize_min = FILESIZE_MIN;
		filesize_max = FILESIZE_MIN;
		blocksize_min = BLOCKSIZE_MIN;
		blocksize_max = BLOCKSIZE_MIN;

		//Verbosely Debug
		#ifdef DEBUG
		fprintf(stderr, "Father: Warmup: Run benchmark with minimum filesize & blocksize\n");
		#endif
	}
	else
	{
		//Verbosely Debug
		#ifdef DEBUG
		fprintf(stderr, "Father: Benchmark: Run real benchmark\n");
		#endif
	}

	#ifdef DEBUG
	fprintf(stderr, "Father: Processes: %d\n", processes);
	fprintf(stderr, "Father: Threads: %d\n", threads);
	#endif

// The Benchmark Operations Section

	//Preliminary Synching is good
	sync();

	//The File size cycle
	for(filesize = filesize_min; filesize <= filesize_max; filesize *= 2)
	{
		//Verbosely Debug
		#ifdef DEBUG
		fprintf(stderr, "Father: File size: %lu K, %lu M, %lu G\n", filesize, filesize/1024, filesize/(1024*1024));
		#endif

		//Prints testing information on screen
		fprintf(stderr, "\n [TEST] File Size %lu K\n",filesize);
	
		//The block size cycle
		for(blocksize = blocksize_min; blocksize <= blocksize_max && blocksize <= filesize; blocksize *= 2)
		{
			//Verbosely Debug
			#ifdef DEBUG
			fprintf(stderr, "Father: Block size: %lu K, %lu M\n", blocksize, blocksize/1024);
			#endif

			//Prints testing information on screen
			fprintf(stderr, "		Testing block size %lu K\n",blocksize);

			//Process group cycle
			for(process_group = 1; process_group <= processes; process_group += 5)
			{
				if(opts->mode == 6 || opts->mode == 7)
					process_group = processes;

				//Verbosely Debug
				#ifdef DEBUG
				fprintf(stderr, "Father: Running benchmark with %d processes\n", process_group);
				#endif
				
				#ifdef WINDOWS
				//cygwin doesn't support this kind of semphores. noop.
				#else
	
				//In the case of multi process mode, the process semaphore start
				if(process_group > 1)
				{
					sem_ident = binary_semaphore_allocation(IPC_PRIVATE, O_CREAT | S_IRWXU);
					
					//The process semaphore inizialization
					//remember process_group+1 to avoid deadlock!
					if(binary_semaphore_initialize(sem_ident, process_group + 1) == (-1))
						fprintf(stderr, "error initializing process semaphore.");
				}
				#endif
				
				//Processes cycle
				for(process = 0; process < process_group; process++)
				{
					// Here the programs generate processes for the multi process mode
					pid = fork();
					if(pid == -1)
					{
						//Managing Errors
						fprintf(stderr, "Father: Error forking child #%d\nFunction fork(), errno = %d: %s\n", process, errno, strerror(errno));
						exit(EXIT_FAILURE);
					}
					else if(pid == 0)
					{
// The Child's code portion
						//Verbosely Debug
						#ifdef DEBUG
						fprintf(stderr, "Child #%d: I'm the child #%d\n", process, process);
						#endif

						sync();

						//Initializing thread_semaphore
						if(threads>1)
							initialize_flag();

						//Thread groups cycle
						for(thread_group = 1; thread_group <= threads; thread_group += 5)
						{
							if(opts->mode == 6 || opts->mode == 7)
								thread_group = threads;

							//Verbosely Debug
							#ifdef DEBUG
							fprintf(stderr, "Child #%d: Running benchmark with %d threads\n", process, thread_group);
							#endif
							
							//Resetting semaphore flags
							if(threads>1)
								reset_flag();
							
							//Threads cycle
							for(thread = 0; thread < thread_group; thread++)
							{
								int result = 0;
		
								//Allocating Memory for Thread Parameters
								thr_par = (struct thread_params*) malloc(sizeof(struct thread_params));
								if(thr_par == NULL) { fprintf(stderr, "Child #%d: Error allocating memory for thr_par array\nFunction malloc(), errno = %d: %s\n", process, errno, strerror(errno)); detach_threads(); clean_child(); exit(EXIT_FAILURE); }
		
								thr_par->mode = opts->mode;
								if(opts->mode == 1)
									thr_par->num_thr_pro = 1;
								else if(opts->mode == 2 || opts->mode == 3)
									thr_par->num_thr_pro = thread_group;
								else if(opts->mode == 4 || opts->mode == 5){
									thr_par->num_thr_pro = process_group;
									thr_par->sem_ident = sem_ident;
								}
								else
									thr_par->num_thr_pro = opts->number;
								thr_par->thread = thread;
								thr_par->process = process;
								thr_par->filesize = filesize;
								thr_par->blocksize = blocksize;
								thr_par->mount_point = opts->mount_point;
								thr_par->gettimes = opts->gettimes;
								thr_par->o_direct = opts->o_direct;
								thr_par->o_sync = opts->o_sync;
								thr_par->posix_rw = opts->posix_rw;
								thr_par->include_flush = opts->include_flush;
								thr_par->cond_var = &thread_flag;
								thr_par->thread_flag_cv = &thread_flag_cv;
								thr_par->thread_flag_mutex = &thread_flag_mutex;
								thr_par->warmup = opts->warmup;

								thrpar[thread] = thr_par;
								result = pthread_create(&tid, NULL, &thread_function, thrpar[thread]);
								//Managing Error
								if(result != 0) { fprintf(stderr, "Child #%d: Error creating thread\nFunction pthread_create(), errno = %d: %s\n", process, errno, strerror(errno)); detach_threads(); clean_child(); exit(EXIT_FAILURE); }

								//Verbosely Debug
								#ifdef DEBUG
								fprintf(stderr, "Child #%d: Thread #%d created\n", process, thread);
								#endif

								tids[thread] = tid;
							}

							//Verbosely Debug
							#ifdef DEBUG
							fprintf(stderr, "Child #%d: All threads created\n", process);
							#endif

							for(thread = 0; thread < thread_group; thread++)
							{
								int result = -1;
								int returned = 0;
		
								result = pthread_join(tids[thread], (void*) &returned);
								if(result != 0) { fprintf(stderr, "Child #%d: Error joining thread\nFunction pthread_join()\n", process); detach_threads(); clean_child(); exit(EXIT_FAILURE); }

								//Verbosely Debug
								#ifdef DEBUG
								fprintf(stderr, "Child #%d: Thread #%d returned value %d\n", process, thread, returned);
								#endif

								if(returned == -1)  { fprintf(stderr, "Child #%d: Thread failed\n", process); detach_threads(); clean_child(); exit(EXIT_FAILURE); }
								returned = 0;
							}

							//Verbosely Debug
							#ifdef DEBUG
							fprintf(stderr, "Child #%d: All threads finished\n", process);
							#endif
		
							if(opts->mode == 2 || opts->mode == 6)
							{
								char filename[25] = "";
								char filepath[1000] = "";
								int result = 0;
		
								if(opts->mode == 2) { sprintf(filename, "/%s_2", RESFILE); }
								if(opts->mode == 6) { sprintf(filename, "/%s_6_%d", RESFILE, getpid()); }
								strcat(filepath, opts->mount_point);
								strcat(filepath, filename);
		
								result = unlink(filepath);
								if(result != 0) { fprintf(stderr, "Child #%d: Error unlinking (deleting) file\nFunction unlink(), errno = %d: %s\n", process, errno, strerror(errno)); }

								//Verbosely Debug
								#ifdef DEBUG
								fprintf(stderr, "Child #%d: File %s unlinked\n", process, filepath);
								#endif
							}
						}

						clean_child();
						exit(EXIT_SUCCESS);
// End of Child's Code Portion
					}
					else
					{
						#ifdef WINDOWS
						//cygwin doesn't support this kind of semphores. noop.
						#else
						//For multi-process test
						if(process_group > 1)
							binary_semaphore_wait(sem_ident);
						#endif

						pids[process] = pid;

						//Verbosely Debug
						#ifdef DEBUG
						fprintf(stderr, "Father: Forked child #%d with PID: %d\n", process, pids[process]);
						#endif
					}
				}

				while(process > 0)
				{
					//Father's waiting
					pid = wait(&status);
					if(pid == -1) { fprintf(stderr, "Father: Error waiting for children termination\nFunction wait(), errno = %d: %s\n", errno, strerror(errno)); exit(EXIT_FAILURE); }

					//Verbosely Debug
					#ifdef DEBUG
					else fprintf(stderr, "Father: Child with PID=%d terminated %s with status %d\n", pid, WIFEXITED(status) != 0 ? "normally" : "abnormally", WIFEXITED(status) != 0 ? WEXITSTATUS(status) : -1000);
					#endif

					//Decrementig Childs-to-wait counter
					process--;
				}
				
				#ifdef WINDOWS
				//cygwin doesn't support this kind of semphores. noop.
				#else
                //Deallocating the proc semaphore
				if(process_group > 1)
					binary_semaphore_deallocate(sem_ident);
				#endif
				
				//Benchmark Mode 4 addictional management
				if(opts->mode == 4)
				{
					char filename[25] = "";
					char filepath[1000] = "";
					int result = 0;

					sprintf(filename, "/%s_4", RESFILE);
					strcat(filepath, opts->mount_point);
					strcat(filepath, filename);

					result = unlink(filepath);
					if(result != 0) { fprintf(stderr, "Father: Error unlinking (deleting) file\nFunction unlink(), errno = %d: %s\n", errno, strerror(errno)); }

					//Verbosely Debug
					#ifdef DEBUG
					fprintf(stderr, "Father: File %s unlinked\n", filepath);
					#endif
				}
	
			//Synching before go on, is a good idea
			sync();
			}
		}
	}

// Write Results Section

	if(opts->warmup == 0)
		//Calling for Assemble CSV Output file in output.c
		assemble_csv_output_file(opts);
	else
		opts->warmup = 0;

// Final Operations Section

	return;
} 
