#!/bin/sh
MAXFSIZE=17000
NUMBER=20
MOUNTPOINT=/xfs

rm -r results
rm $MOUNTPOINT/benchIO*

# ANSI test
./benchIO -m $MOUNTPOINT -M 1 -S $MAXFSIZE -o benchIO_$(date +%F)_$(date +%H-%M-%S).csv ; rm -r results

# POSIX test
./benchIO -m $MOUNTPOINT -M 1 -S $MAXFSIZE -o benchIO_$(date +%F)_$(date +%H-%M-%S).csv -p ; rm -r results

# POSIX with O_SYNC
./benchIO -m $MOUNTPOINT -M 1 -S $MAXFSIZE -o benchIO_$(date +%F)_$(date +%H-%M-%S).csv -p -s ; rm -r results

# POSIX with O_DIRECT
./benchIO -m $MOUNTPOINT -M 1 -S $MAXFSIZE -o benchIO_$(date +%F)_$(date +%H-%M-%S).csv -p -d ; rm -r results

# ANSI multi-thread single-file
./benchIO -m $MOUNTPOINT -M 2 -n $NUMBER -S $MAXFSIZE -o benchIO_$(date +%F)_$(date +%H-%M-%S).csv ; rm -r results

# ANSI multi-thread multi-file
./benchIO -m $MOUNTPOINT -M 3 -n $NUMBER -o benchIO_$(date +%F)_$(date +%H-%M-%S).csv ; rm -r results

# ANSI multi-process single-file
./benchIO -m $MOUNTPOINT -M 4 -n $NUMBER -S $MAXFSIZE -o benchIO_$(date +%F)_$(date +%H-%M-%S).csv ; rm -r results

# ANSI multi-process multi-file
./benchIO -m $MOUNTPOINT -M 5 -n $NUMBER -o benchIO_$(date +%F)_$(date +%H-%M-%S).csv ; rm -r results

# ANSI multi-thread-process single-file
./benchIO -m $MOUNTPOINT -M 6 -n $NUMBER -S $MAXFSIZE -o benchIO_$(date +%F)_$(date +%H-%M-%S).csv ; rm -r results

# ANSI multi-thread-process multi-file
./benchIO -m $MOUNTPOINT -M 7 -n $NUMBER -o benchIO_$(date +%F)_$(date +%H-%M-%S).csv ; rm -r results
