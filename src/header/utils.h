// [benchIO] HEADER FILE
//
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File utils.h
// Last Revision:   23th June, 2005

#include "main.h"

int I_am_root();
char *get_line_from_file(FILE *file);
char *mnt_sanity_check(char *mount);
void set_process_sched(int set_value, int prio);


