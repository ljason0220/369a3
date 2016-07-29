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

/* ext2_cp: This program takes three command line arguments. 1. name of an 
ext2 formatted virtual disk; 2. path to a file on your native operating 
system; and 3. an absolute path on your ext2 formatted disk. The program 
should work like cp. If the specified file or target location does not exist, 
then your program should  return the appropriate error (ENOENT). */