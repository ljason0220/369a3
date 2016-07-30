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
#include <libgen.h>

/* ext2_ln: This program takes three command line arguments. Eirst is name of 
an ext2 formatted virtual disk. The other two are absolute paths on your 
ext2 formatted disk. The program should work like ln, creating a link from 
the first specified file to the second specified path. If the source file 
does not exist (ENOENT), if the link name already exists (EEXIST), or if 
either location refers to a directory (EISDIR), then your program should 
return the appropriate error. */