// [benchIO] HEADER FILE
//
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File output.h
// Last Revision:   23th June, 2005

#include "main.h"

int write_options_info_on_file(struct options* opts);
int read_options_info_from_file(struct options** opts);
int write_result_on_file(struct thread_params* tp, struct test_result* result);
int assemble_csv_output_file(struct options* opts);
