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

int main(int argc, char **argv) {

  if(argc != 3) {
    fprintf(stderr, "Usage: readimg <image file name> <absolute path>\n");
    exit(1);
    }

    int fd = open(argv[1], O_RDWR);
 //    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
 //  	if(disk == MAP_FAILED) {
 //  		perror("mmap");
 //    	exit(1);
	// }

	read_disk(fd)

	char *path = argv[2];
	char *absolute;
	absolute = strtok(path, "/");

	


}