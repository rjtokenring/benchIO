// [benchIO]
//
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File output.c
// Last Revision:   23th June, 2005
//
// output.c :
// 1) temporary result files writing and reading
// 2) writing final output csv file from results

#include "header/main.h"

/*
//This function writes the test options in a binary file. The files is written in the results dir
int write_options_info_on_file(struct options* opts)
{
	FILE* pointer;
	char filename[50] = "";
	int ret = 0;
	//Binary writing
	snprintf(filename, 50, "%s/bench_options", RESDIR);
	pointer = fopen(filename, "w");
	//Error opening the file
	if(pointer == NULL)
		{ fprintf(stderr, "Error opening file %s/bench_options.\n", RESDIR); return -1; }
	//Error writing on file
	if(fwrite(opts, sizeof(struct options), 1, pointer) <= 0)
		{ fprintf(stderr,"Error writing file %s/bench_options.\n", RESDIR); ret = -1; }
	//FFlushing anc closing the FD
	fflush(pointer);
	fclose(pointer);
	// Returning the value 0 if all correct
	return ret;
}
*/

/*
//This function retrieves option from the binary file the describe the test options
int read_options_info_from_file(struct options** opts)
{
	FILE* pointer;
	char filename[50] = "";
	int ret = 0;
	//Binary reading the file
	snprintf(filename, 50, "%s/bench_options", RESDIR);
	pointer = fopen(filename, "r");
	//Error opening the file
	if(pointer == NULL)
		{ fprintf(stderr, "Error opening file %s/bench_options.\n", RESDIR); return -1; }
	//Error reading the file
	if(fread(*opts, sizeof(struct options), 1, pointer) <= 0)
		{ fprintf(stderr, "Error reading file %s/bench_options.\n", RESDIR); ret = -1; }
	//Closing the Fd
	fclose(pointer);
	//Returning the value 0 if all correct
	return ret;
}
*/

//This function writes on a file the results from a single test 
int write_result_on_file(struct thread_params* tp, struct test_result* result)
{
	FILE* pointer;
	char filename[500] = "";
	int ret = 0;
	
	//Start opening files
	//The filename convention: filename_#filesize_#blocksize_#group_#process_#thread es. test_512_4_0_1_1
	snprintf(filename, 500, "%s/%s_%lu_%lu_%d_%d_%d", RESDIR, RESFILE, tp->filesize, tp->blocksize, tp->num_thr_pro, tp->process, tp->thread);

	pointer = fopen(filename, "w");
	//Error opening the file
	if(pointer == NULL)
		{ fprintf(stderr, "Error opening file %s.\n", filename); return -1; }
	//Error writing the file
	if(fwrite(result, sizeof(struct test_result), 1, pointer) <= 0)
		{ fprintf(stderr,"Error writing file %s.\n", filename); ret = -1; }
	//FFlushing and closing the FD
	fflush(pointer);
	fclose(pointer);
	//Returning the value 0 if all correct
	return ret;
}

//This functions retrives all the output files of the whole test and merge them into a Comma Separated Values file
//It makes a preliminary TMP file with all the data tabbed with the benchIO convenction for Output then it makes a CVS file, it heads it with all the info about the test, and last it move all data from TMP to CSV file
//Called once at the end of benchmark() function in process.c
int assemble_csv_output_file(struct options* opts)
{
	FILE* tmp = NULL;
	FILE* csv = NULL;
	FILE* ptr_test = NULL;
	char filename_test[500] = "";
	struct test_result test;
	//Featuring Vars to manages the outputs files
	unsigned long int filesize = 0;
	unsigned long int filesize_min = 0;
	unsigned long int filesize_max = 0;
	unsigned long int blocksize = 0;
	unsigned long int blocksize_min = 0;
	unsigned long int blocksize_max = 0;
	int process_group = 0;
	int thread_group = 0;
	int processes = 1;
	int process = 0;
	int threads = 1;
	int thread = 0;
	int ret = 0;
	int i = 0;
	char a = 0, b = 0;

	tmp = fopen("tmp", "w");
	if(tmp == NULL)
		{ fprintf(stderr, "Error opening file %s\n", "tmp"); return -1; }

	for(i = 0; i < 20; i++)
	{
		//Labelling Tests
		if(i == 0) fprintf(tmp, "Sequential Write Total Time\n");
		if(i == 1) fprintf(tmp, "Sequential Write Throughput\n");
		if(i == 2) fprintf(tmp, "Sequential Write User Time\n");
		if(i == 3) fprintf(tmp, "Sequential Write System Time\n");
		if(i == 4) fprintf(tmp, "Sequential Write CPU Usage\n");
		if(i == 5) fprintf(tmp, "Sequential Read Total Time\n");
		if(i == 6) fprintf(tmp, "Sequential Read Throughput\n");
		if(i == 7) fprintf(tmp, "Sequential Read User Time\n");
		if(i == 8) fprintf(tmp, "Sequential Read System Time\n");
		if(i == 9) fprintf(tmp, "Sequential Read CPU Usage\n");
		if(i == 10) fprintf(tmp, "Random Write Total Time\n");
		if(i == 11) fprintf(tmp, "Random Write Throughput\n");
		if(i == 12) fprintf(tmp, "Random Write User Time\n");
		if(i == 13) fprintf(tmp, "Random Write System Time\n");
		if(i == 14) fprintf(tmp, "Random Write CPU Usage\n");
		if(i == 15) fprintf(tmp, "Random Read Total Time\n");
		if(i == 16) fprintf(tmp, "Random Read Throughput\n");
		if(i == 17) fprintf(tmp, "Random Read User Time\n");
		if(i == 18) fprintf(tmp, "Random Read System Time\n");
		if(i == 19) fprintf(tmp, "Random Read CPU Usage\n");

		//Features of the tests like filesize and blocksize MAX and MIN
		filesize_min = FILESIZE_MIN;
		filesize_max = opts->maxfsize;
		blocksize_min = BLOCKSIZE_MIN;
		blocksize_max = BLOCKSIZE_MAX;

		//Setting up Featuring Vars for Mode 1 test
		if(opts->mode == 1){ processes = 1; threads = 1; }
		//Setting up Featuring Vars for Mode 2 test
		if(opts->mode == 2){ processes = 1; threads = opts->number; blocksize_min = BLOCKSIZE_MAX_MULTI; blocksize_max = BLOCKSIZE_MAX_MULTI; }
		//Setting up Featuring Vars for Mode 3 test
		if(opts->mode == 3){ processes = 1; threads = opts->number; filesize_max = FILESIZE_MAX_MULTI; blocksize_min = BLOCKSIZE_MAX_MULTI; blocksize_max = BLOCKSIZE_MAX_MULTI; }
		//Setting up Featuring Vars for Mode 4 test
		if(opts->mode == 4){ processes = opts->number; threads = 1; blocksize_min = BLOCKSIZE_MAX_MULTI; blocksize_max = BLOCKSIZE_MAX_MULTI; }
		//Setting up Featuring Vars for Mode 5 test
		if(opts->mode == 5){ processes = opts->number; threads = 1; filesize_max = FILESIZE_MAX_MULTI; blocksize_min = BLOCKSIZE_MAX_MULTI; blocksize_max = BLOCKSIZE_MAX_MULTI; }
		//Setting up Featuring Vars for Mode 6 test
		if(opts->mode == 6){ processes = opts->number; threads = opts->number; filesize_max = FILESIZE_MAX_MULTI; blocksize_min = BLOCKSIZE_MAX_MULTI; blocksize_max = BLOCKSIZE_MAX_MULTI; }
		//Setting up Featuring Vars for Mode 7 test
		if(opts->mode == 7){ processes = opts->number; threads = opts->number; filesize_max = FILESIZE_MAX_MULTI; blocksize_min = BLOCKSIZE_MAX_MULTI; blocksize_max = BLOCKSIZE_MAX_MULTI; }
	

		//THE CSV FILE MERGING ENGINE
		//This engine runs like benchmark engine

		//Cycling filesizes by 2-multiples
		for(filesize = filesize_min; filesize <= filesize_max; filesize *= 2)
		{
			//Cycling blocksizes by 2-multiples
			for(blocksize = blocksize_min; blocksize <= blocksize_max && blocksize <= filesize; blocksize *= 2)
			{
				//Cycling process_group by 5 processes step
				for(process_group = 1; process_group <= processes; process_group += 5)
				{
					if(opts->mode == 6 || opts->mode == 7)
						process_group = processes;
					//Cycling processes
					for(process = 0; process < process_group; process++)
					{
						//Cycling thread_group by 5 processes step
						for(thread_group = 1; thread_group <= threads; thread_group += 5)
						{
							if(opts->mode == 6 || opts->mode == 7)
								thread_group = threads;
							//Cycling threads
							for(thread = 0; thread < thread_group; thread++)
							{
								snprintf(filename_test, 500, "%s/%s_%lu_%lu_%d_%d_%d", RESDIR, RESFILE, filesize, blocksize, process_group >= thread_group ? process_group : thread_group, process, thread);
								//Error opening file
								ptr_test = fopen(filename_test, "r");
								if(ptr_test == NULL)
									{ fprintf(stderr, "Error opening file %s.\n", filename_test); ret = -1; break; }
								//Error writing
								if(fread(&test, sizeof(struct test_result), 1, ptr_test) <= 0)
									{ fprintf(stderr, "Error reading file %s.\n", filename_test); ret = -1; }
	
								fclose(ptr_test);
								//Building all the file needed by each modality (See above for the "i" var values
								if(i == 0) fprintf(tmp, "%f", test.swt);
								if(i == 1) fprintf(tmp, "%f", (double)filesize / test.swt);
								if(i == 2) fprintf(tmp, "%f", test.swu);
								if(i == 3) fprintf(tmp, "%f", test.sws);
								if(i == 4) fprintf(tmp, "%f", test.swc);
								if(i == 5) fprintf(tmp, "%f", test.srt);
								if(i == 6) fprintf(tmp, "%f", (double)filesize / test.srt);
								if(i == 7) fprintf(tmp, "%f", test.sru);
								if(i == 8) fprintf(tmp, "%f", test.srs);
								if(i == 9) fprintf(tmp, "%f", test.src);
								if(i == 10) fprintf(tmp, "%f", test.rwt);
								if(i == 11) fprintf(tmp, "%f", (double)filesize / test.rwt);
								if(i == 12) fprintf(tmp, "%f", test.rwu);
								if(i == 13) fprintf(tmp, "%f", test.rws);
								if(i == 14) fprintf(tmp, "%f", test.rwc);
								if(i == 15) fprintf(tmp, "%f", test.rrt);
								if(i == 16) fprintf(tmp, "%f", (double)filesize / test.rrt);
								if(i == 17) fprintf(tmp, "%f", test.rru);
								if(i == 18) fprintf(tmp, "%f", test.rrs);
								if(i == 19) fprintf(tmp, "%f", test.rrc);

								//insert comma to separate threads
								if(opts->mode == 2 || opts->mode == 3 || opts->mode == 6 || opts->mode == 7) fprintf(tmp, ",");
							}
							//Insert pipe to separate thread groups
							if(opts->mode == 2 || opts->mode == 3) fprintf(tmp, "|");
						}
						//insert comma or pipe to separate different processes
						if(opts->mode == 4 || opts->mode == 5) fprintf(tmp, ",");
						if(opts->mode == 6 || opts->mode == 7) fprintf(tmp, "|");
					}
					//insert pipe to separate process groups
					if(opts->mode == 4 || opts->mode == 5) fprintf(tmp, "|");
				}
				//insert comma to separate different blocksizes
				if(opts->mode == 1) fprintf(tmp, ",");
			}
			//insert newline to separate different filesizes
			fprintf(tmp, "\n");
		}
		//Line Terminator char and return for a new test
		fprintf(tmp, "\n");
		fprintf(tmp, "\n");
	}
	//closing the temp file
	fclose(tmp);
	//Reopening tmp in readonly
	tmp = fopen("tmp", "r");
	if(tmp == NULL)
		{ fprintf(stderr, "Error opening file %s.\n", "tmp"); return -1; }
	csv = fopen(opts->outfile, "w");
	if(csv == NULL)
		{ fprintf(stderr, "Error opening file %s.\n", opts->outfile); return -1; }
	

	//Heading the CSV file with all the interesting info about benchmark option
	fprintf(csv, "Mount Point: %s\n", opts->mount_point);
	fprintf(csv, "Mode: %d\n", opts->mode);
	fprintf(csv, "Number of threads/processes: %d\n", opts->number);
	if(opts->mode == 1)
	{
		fprintf(csv, "Filesizes: ");
		for(filesize = filesize_min; filesize <= filesize_max; filesize *= 2)
			fprintf(csv, "%lu ", filesize);
		fprintf(csv, "\n");
	}
	else
		fprintf(csv, "Filesize: 4 Mb\n");
	fprintf(csv, "System function used to measure times: %s\n", opts->gettimes == 0 ? "gettimeofday" : (opts->gettimes == 1 ? "times" : "rtc"));
	fprintf(csv, "I/O Access: %s", opts->posix_rw == 0 ? "ANSI C buffered" : "POSIX");
	if(opts->o_sync == 1)
		fprintf(csv, " with O_SYNC");
	if(opts->o_direct == 1)
		fprintf(csv, " with O_DIRECT");
	fprintf(csv, "\n");
	if(opts->vfs != 0)
		fprintf(csv, "VSF cache pressure: %d\n", opts->vfs);
	if(opts->dirty != 0)
		fprintf(csv, "Dirty Background ratio: %d\n", opts->dirty);
	if(opts->mode > 1)
		fprintf(csv, "Process scheduler: %s\n", opts->prsched == 0 ? "time sharing" : (opts->prsched == 1 ? "round robin" : "FIFO"));
	if(opts->iosched >= 0)
		fprintf(csv, "I/O scheduler: %s\n", opts->iosched == 0 ? "anticipatory" : (opts->iosched == 1 ? "cfq" : (opts->iosched == 2 ? "noop" : "deadline")));
	else
		fprintf(csv, "I/O scheduler: system default\n");
	if(opts->lock_memory != 0)
		fprintf(csv, "Memory locked\n");
	fprintf(csv, "Notes:\n");
	fprintf(csv, "\n");
	fprintf(csv, "\n");
	
	//Move all data from tmp to CSV definive file, filtering bad separator combinations
	do
	{
		b = a;
		a = (char) fgetc(tmp);
		if(b && !(((b==',' || b=='|') && a=='\n') || (b==',' && a=='|'))) fputc((char) b, csv);
	} while(!feof(tmp));

	fclose(csv);
	fclose(tmp);
	//Unlink the tmp file
	unlink("tmp");

	//Returnig value 0 if everything goes right
	return ret;
}
