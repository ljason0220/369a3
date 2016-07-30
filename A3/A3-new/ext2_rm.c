#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include "helper.h"
#include <string.h>
#include <errno.h>

/* ext2_rm: This program takes two command line arguments. 1. name of an ext2 
formatted virtual disk; and 2. an absolute path to a file or link (not a 
directory) on that disk. The program should work like rm, removing the 
specified file from the disk. If the file does not exist or if it is a 
directory, then your program should return the appropriate error. */