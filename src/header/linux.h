// [benchIO] HEADER FILE
//
// a POSIX ANSI C multi-platform multi-function benchmark for Input/Output subsystems
//
// written and developed by (in alphabetical order) Bostrenghi Federico, Colombo Marco and Greco Marcello
// distributed under GPL license - 2005
//
// Program Version:  0.1_devel
// File linux.h
// Last Revision:   23th June, 2005

#include "main.h"

void linux_get_filesystem_info(char* mount_point);
char *parse_line(char* char_line);
void mdstat_parse_line(char* char_line);
char *get_dev_by_mnt(char *mount_point);
void get_system_env(char *mount_point);
void restore_system_env();
void set_IO_sched(char *set_value);
void set_vfs_cache_pressure(int pres);
void set_dirty_background_ratio(int pres);
void pvscan_parse_line(FILE *fd);
int lock_memory(void);
int unlock_memory(void);



