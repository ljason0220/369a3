#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include <string.h>

/* ext2_ls: This program takes two command line arguments. Name of an ext2 
formatted virtual disk and an absolute path on the ext2 formatted disk. The 
program should work like ls -1. If the flag "-a" is specified (after the 
disk image argument), your program should also print the . and .. entries. 
If the path does not exist, print "No such file or directory", and return an 
ENOENT. 
*/