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

/* ext2_mkdir: This program takes two command line arguments. 1. name of an 
ext2 formatted virtual disk; and 2. an absolute path on your ext2 formatted 
disk. The program should work like mkdir, creating the final directory on 
the specified path on the disk. If any component on the path to the location 
where the final directory is to be created does not exist or if the specified 
directory already exists, then your program should return the appropriate 
error (ENOENT or EEXIST). */