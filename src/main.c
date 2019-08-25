// [benchIO]
//
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File main.c
// Last Revision:   23th June, 2005
//
// main.c :
// 1) printing usage
// 2) setting default options
// 3) command line options parsing
// 4) options' coherence check
// 5) warmup and real benchmark

#include "header/main.h"

//Var declaration for opts (options) 
const char* program_name = NULL;
struct options opts;

// Function print_usage 
// when invoked print on passed file stream the usage of the program
void print_usage(FILE* stream)
{
	fprintf(stream, "Usage: %s options\n", program_name);
	fprintf(stream,
		"General Options:\n"
		// Help
		"	-h	--help              Display this usage information.\n"
		// Verbose mode
		"	-v	--verbose           Print verbose messages.\n"
		// Defines the mount point
		"	-m	--mount-point=mp    Run tests on file system mounted in <mp>.\n"
		// Defines the Test Mode 1-7, description of the modes can be read under
		"	-M	--mode=n            Run tests in mode:\n"
		"			1 = single process, single thread (default, default 512 Mb)\n"
		"			2 = single process, multi thread (1 file for all threads, default 512 Mb)\n"
		"			3 = single process, multi thread (1 file for each thread, 4 Mb)\n"
		"			4 = multi process (1 file for all processes, default 512 Mb)\n"
		"			5 = multi process (1 file for each process, 4 Mb)\n"
		"			6 = multi process, multi thread (1 file for all threads of the same process, 4 Mb)\n"
		"			7 = multi process, multi thread (1 file for each thread, 4 Mb)\n"
		"			Modes > 1 use 4 Kb as fixed blocksize.\n"
		"			Pay attention to have enough free disk space in the mounted file system.\n"
		// Defines the concurrent number of threads and/or processes used in the test. Please note MAX VALUE
		"	-n	--number=n          Number of threads/processes (default = %d, max = 1000).\n"
		// Defines the max filesize of the test. The test will run from a MIN Kb file to a MAXFSIZE Kb file.
		// For each iteration filesize is doubled: 4-8-16-32...
		"	-S	--maxfsize=n        Maximum filesize in Kb: min = 4, max = 2*1024^2 = 2097152,\n"
		"							default = 512*1024 = 524288.\n"
		// Defines the output file of the benchmark
		"	-o	--output=of         Write output of benchmark in a file named <of> (default: %s).\n"
		// Defines the time method of the benchmark
		"	-t	--gettimes=n        Method to use to get timings:\n"
					// Uses gettimeofday() (setted as default time method)
		"			0 = gettimeofday() (default)\n"
					// Uses times()
		"			1 = times()\n"
					// Uses Real Time Clock in Linux systems
		"			2 = RTC (Linux only, root only)\n"
		// Defines the POSIX O_SYNC method to write on IO subsystem (where available)
		"	-s	--sync              Use POSIX O_SYNC.\n"
		// Defines the option to run the test with POSIX I/O filesystem access functions instead of ANSI C Buffered function
		"	-p	--posix             Use POSIX I/O functions open(), read(), write(), close() instead of\n"
		"		                        ANSI C buffered fopen(), fread(), fwrite(), fclose().\n"
		//Defines the inclusion of the flush and close operations in the time measurements.
		"	-F	--flush             Include opening, flushing and closing times in measurements (default = yes).\n"
		"\n"
		// Defining here addiction functions only for Linux Systems supporting these features (2.6.x)
		"Linux Specific Options:\n"
		// Defines the POSIX O_DIRECT method to write on IO subsystem (where available)
		"	-d	--direct            Use POSIX O_DIRECT (requires -p).\n"
		//Virtual File System Cache Pressure settings
		"	-V	--vfs=n             Change VFS cache pressure (0 - 1000).\n"
		//Dirty Background Ration settings
		"	-D	--dirty=n           Change dirty background ratio (0 - 100).\n"
		//Processes scheduler settings
		"	-P	--prsched=n         Change process scheduler:\n"
		"			0 = Time Sharing (default)\n"
		"			1 = Round Robin\n"
		"			2 = FIFO\n"
		//Process priority settings
		"	-R	--priority=n        Change process priority (depending from scheduler and user rights).\n"
		//I/O Scheduler settings
		"	-I	--iosched=n         Change I/O scheduler:\n"
		"			0 = anticipatory\n"
		"			1 = cfq\n"
		"			2 = noop\n"
		"			3 = deadline\n"
		//Setting up the lock memory function for process
		"	-L	--lock-memory       Lock process memory (current and future)\n",
	DEFAULT_NUM_PT, CSVFILE);
}

//Defines opts (options) default values
void option_defaults()
{
	opts.verbose = 0;
	opts.mount_point = "";
	opts.mode = 1;
	opts.number = DEFAULT_NUM_PT;
	opts.maxfsize = FILESIZE_MAX;
	opts.outfile = CSVFILE;
	opts.gettimes = 0;
	opts.o_direct = 0;
	opts.o_sync = 0;
	opts.posix_rw = 0;
	opts.vfs = 0;
	opts.dirty = 0;
	opts.prsched = 0;
	opts.priority = 1;
	opts.iosched = -1;
	opts.rtcfreq = RTC_FREQUENCY;
	opts.include_flush = 1;
	opts.lock_memory = 0;
	opts.warmup = 1;
}

// Check some critical errors or option incompatibility in command line arguments
void check()
{
	if(strcmp(opts.mount_point, "") == 0)
	{
		fprintf(stderr, "[EE] Mount point not defined!\n");
		exit(EXIT_FAILURE);
	}
	if(opts.mode == 1)
		opts.number = 1;
	if((opts.mode >= 2 && opts.mode <= 7) && opts.number == 1)
	{
		fprintf(stderr, "[I] The number of threads/processes for mode 2, 3, 4, 5, 6, 7 (multi thread/process) must be greater than 1!\n");
		exit(EXIT_FAILURE);
	}
	#ifdef LINUX
	if(opts.posix_rw == 0 && opts.o_direct == 1)
	{
		fprintf(stderr, "[I] POSIX O_DIRECT flag can only be used with POSIX I/O! Assuming o_direct = 0.\n");
		opts.o_direct = 0;
	}
	#endif
	if(opts.posix_rw == 0 && opts.o_sync == 1)
	{
		fprintf(stderr, "[I] POSIX O_SYNC flag can only be used with POSIX I/O! Assuming o_sync = 0.\n");
		opts.o_sync = 0;
	}
	#ifdef LINUX
	if(opts.o_direct == 1 && opts.o_sync == 1)
	{
		fprintf(stderr, "[I] POSIX O_SYNC and O_DIRECT flags are mutually exclusive (O_DIRECT already works in synchronous mode)! Assuming o_sync = 0.\n");
		opts.o_sync = 0;
	}
	#endif
	#ifdef LINUX
	if(opts.gettimes == 2 && opts.mode != 1)
	{
		fprintf(stderr, "[I] RTC is only available in single process, single thread mode (Mode 1)! Assuming gettimes = 0 (gettimeofday).\n");
		opts.gettimes = 0;
	}
	#endif
}

// The benchIO Main


int main(int argc, char* argv[])
{
// Main variables

	//Opts indexing vars
	int next_option;
	extern char *optarg;
	extern int optind;

	// Short options string
	const char* short_options = "hvm:M:n:S:o:t:dspV:D:P:R:I:FL";

	// Long options structure
	const struct option long_options[] = {
		{"help",		0,	NULL,	'h'},
		{"verbose",		0,	NULL,	'v'},
		{"mount-point",		1,	NULL,	'm'},
		{"mode",		1,	NULL,	'M'},
		{"number",		1,	NULL,	'n'},
		{"maxfsize",		1,	NULL,	'S'},
		{"output",		1,	NULL,	'o'},
		{"gettimes",		1,	NULL,	't'},
		{"direct",		0,	NULL,	'd'},
		{"sync",		0,	NULL,	's'},
		{"posix",		0,	NULL,	'p'},
		{"vfs",			1,	NULL,	'V'},
		{"dirty",		1,	NULL,	'D'},
		{"prsched",		1,	NULL,	'P'},
		{"priority",		1,	NULL,	'R'},
		{"iosched",		1,	NULL,	'I'},
		{"flush",		0,	NULL,	'F'},
		{"lock-memory",		0,	NULL,	'L'},
		{NULL,			0,	NULL,	0}};

// Startup Operations

	// Fetching the program name from argv[0]
	program_name = argv[0];
	if(program_name && strrchr (program_name, '/'))
		program_name = strrchr (program_name, '/') + 1;

	
	// Prints the program usage if the argument counter is smaller than 2
	if(argc < 2)
	{
		print_usage(stdout);
		exit(EXIT_SUCCESS);
	}

	// Sets default values for options
	option_defaults();

	//write banner
	fprintf(stderr,"\n\n[BenchIO] a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems\n\n");

// Option Operations

	do
	{
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);
		switch(next_option)
		{
			// Help implementation
			case 'h':
				print_usage(stdout);
				exit(EXIT_SUCCESS);
			// Verbose mode implementation
			case 'v':
				opts.verbose = 1;
				break;
			// Mount point option implementation
			case 'm':
				opts.mount_point = mnt_sanity_check(optarg);
				break;
			// Benchmarks mode implementations and check if mode 1<=M<=7
			case 'M':
				opts.mode = atoi(optarg);
				if(opts.mode < 1 || opts.mode > 7) { fprintf(stderr, "[EE] Options: Invalid mode!\n"); exit(EXIT_FAILURE); }
				break;
			// Number of thread option definition 
			case 'n':
				opts.number = atoi(optarg);
				if(opts.number < 2 || opts.number > 1000) { fprintf(stderr, "[EE] Options: Invalid number of threads/processes!\n"); exit(EXIT_FAILURE); }
				break;
			// MAXFILESIZE option definition 
			case 'S':
				opts.maxfsize = atoi(optarg);
				if(opts.maxfsize < FILESIZE_MIN || opts.maxfsize > FILESIZE_LIMIT) { fprintf(stderr, "[EE] Options: Invalid maximum file size!\n"); exit(EXIT_FAILURE); }
				break;
			// Output File option definition
			case 'o':
				opts.outfile = optarg;
				if(strcmp(opts.outfile, "") == 0) { fprintf(stderr, "[EE] Options: Invalid output file name!\n"); exit(EXIT_FAILURE); }
				break;
			// Timing Method option definition
			case 't':
				opts.gettimes = atoi(optarg);
				if(opts.gettimes < 0 || opts.gettimes > 2) { fprintf(stderr, "[EE] Options: Invalid method to get timings!\n"); exit(EXIT_FAILURE); }
				#ifndef LINUX
				if(opts.gettimes == 2) { fprintf(stderr, "[+] Options: RTC is only for Linux.\n"); exit(EXIT_FAILURE); }
				#endif
				break;
			// O_SYNC definition
			case 's':
				opts.o_sync = 1;
				break;
			// Include flush and close operations in time measurements definition
			case 'F':
				opts.include_flush = 1;
				break;
			// POSIX Filesystem access function definition
			case 'p':
				opts.posix_rw = 1;
				break;

		// Start of LINUX specific options definition
			// O_DIRECT definition
			case 'd':
				#ifdef LINUX
				opts.o_direct = 1;
				#else
				fprintf(stderr, "[+] Options: -d option is only for Linux.\n");
				exit(EXIT_FAILURE);
				#endif
				break;
			// VFS Pressure and option checks 
			case 'V':
				#ifdef LINUX
				opts.vfs = atoi(optarg);
				if(opts.vfs < 0 || opts.vfs > 1000) { fprintf(stderr, "[EE] Options: Invalid vfs cache pressure value!\n"); exit(EXIT_FAILURE); }
				set_vfs_cache_pressure((int)opts.vfs);
				#else
				fprintf(stderr, "[+] Options: -V option is only for Linux.\n");
				exit(EXIT_FAILURE);
				#endif
				break;
			// Dirty Background Ratio and option checks 
			case 'D':
				#ifdef LINUX
				opts.dirty = atoi(optarg);
				if(opts.dirty < 0 || opts.dirty > 100) { fprintf(stderr, "[EE] Options: Invalid dirty background ratio!\n"); exit(EXIT_FAILURE); }
				set_dirty_background_ratio((int)opts.dirty);
				#else
				fprintf(stderr, "[+] Options: -D option is only for Linux.\n");
				exit(EXIT_FAILURE);
				#endif 
				break;
			// Process Scheduler and option checks 
			case 'P':
				#ifdef LINUX
				opts.prsched = atoi(optarg);
				if(opts.prsched < 0 || opts.prsched > 2) { fprintf(stderr, "[EE] Options: Invalid process scheduler!\n"); exit(EXIT_FAILURE); }
				set_process_sched(opts.prsched,opts.priority);
				#else
				fprintf(stderr, "[+] Options: -P option is only for Linux.\n");
				exit(EXIT_FAILURE);
				#endif 
				break;
			// Process Priority
			case 'R':
				#ifdef LINUX
				opts.priority = atoi(optarg);
				set_process_sched(opts.prsched,opts.priority);
				#else
				fprintf(stderr, "[+] Options: -R option is only for Linux.\n");
				exit(EXIT_FAILURE);
				#endif 
				break;
			// I/O Scheduler and option checks 
			case 'I':
				#ifdef LINUX
				opts.iosched = atoi(optarg);
				if(opts.iosched < 0 || opts.iosched > 3) { fprintf(stderr, "[EE] Options: Invalid I/O scheduler!\n"); exit(EXIT_FAILURE); }
				#else
				fprintf(stderr, "[+] Options: -I option is only for Linux.\n");
				exit(EXIT_FAILURE);
				#endif 
				break;
			// Lock memory definition
			case 'L':
				#ifdef LINUX
				opts.lock_memory = 1;
				#else
				fprintf(stderr, "[+] Options: -I option is only for Linux.\n");
				exit(EXIT_FAILURE);
				#endif 
				break;
		// End of Linux Specific
			//Invalid option
			case '?':
				print_usage(stderr);
				exit(EXIT_FAILURE);
			case -1:
				break;
			//The default exits without do nothing
			default:
				exit(EXIT_FAILURE);
		}
	} while (next_option != -1);
	
	//Debug mode additional onscreen outputs	
	#ifdef DEBUG
	fprintf(stderr, "Main: Mount Point: %s\n", opts.mount_point);
	fprintf(stderr, "Main: Mode: %d\n", opts.mode);
	fprintf(stderr, "Main: Number of threads/processes: %d\n", opts.number);
	fprintf(stderr, "Main: Max filesize: %d\n", opts.maxfsize);
	fprintf(stderr, "Main: Output file: %s\n", opts.outfile);
	fprintf(stderr, "Main: Method to get timings: %d\n", opts.gettimes);
	fprintf(stderr, "Main: Use O_DIRECT: %d\n", opts.o_direct);
	fprintf(stderr, "Main: Use O_SYNC: %d\n", opts.o_sync);
	fprintf(stderr, "Main: Use POSIX I/O Functions: %d\n", opts.posix_rw);
	fprintf(stderr, "Main: Vfs cache pressure: %d\n", opts.vfs);
	fprintf(stderr, "Main: Dirty background ratio: %d\n", opts.dirty);
	fprintf(stderr, "Main: Process scheduler: %d\n", opts.prsched);
	fprintf(stderr, "Main: Priority: %d\n", opts.priority);
	fprintf(stderr, "Main: I/O scheduler: %d\n", opts.iosched);
	fprintf(stderr, "Main: RTC frequency: %d\n", opts.rtcfreq);
	if (opts.verbose)
	{
		// On-Screen Main arguments debug output
		int i;
		for(i = optind; i < argc; ++i) fprintf(stderr, "Main: Argument: %s\n", argv[i]);
	}
	#endif	

// Environment Startup

	//Check on options
	check();

	//Get min system time resolution
	get_resolution(opts.gettimes);

	#ifdef LINUX
	//Saving current system settings
	get_system_env(opts.mount_point);
	
	//Setting up the I/O Scheduler if requested
	if(opts.iosched >= 0)
	{
		if(opts.iosched == 0) set_IO_sched("anticipatory");
		if(opts.iosched == 1) set_IO_sched("cfq");
		if(opts.iosched == 2) set_IO_sched("noop");
		if(opts.iosched == 3) set_IO_sched("deadline");
	}
	
	//Locking the memory if requested	
	if(opts.lock_memory)
		lock_memory();
	#endif

	//Create directory to store measurements' results
	if(mkdir(RESDIR, 00700) == 0)
	{
		//The program warmup. See process.c file for the benchmark function
		benchmark(&opts);
	
		//The real benchmark starts here. See process.c file for the benchmark function
		benchmark(&opts);

		//Prints testing information on screen
		fprintf(stderr, "\n [All tests completed] \n\n");
	}
	else
	{
		fprintf(stderr, "[EE] Error creating directory '%s' (maybe already exists? move, remove or rename it.)\n", RESDIR);
		#ifdef DEBUG
		fprintf(stderr, "[+] Function mkdir(), errno = %d: %s\n", errno, strerror(errno));
		#endif
	}
	
	#ifdef LINUX
	//Unlocking process memory if lock enabled, before exiting
	if(opts.lock_memory)
		unlock_memory();

	//Restoring system settings before exiting
	restore_system_env();
	#endif

// Final Operations

	return 0;
}
