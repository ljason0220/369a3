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

/* ext2_ls: This program takes two command line arguments. Name of an ext2 
formatted virtual disk and an absolute path on the ext2 formatted disk. The 
program should work like ls -1. If the flag "-a" is specified (after the 
disk image argument), your program should also print the . and .. entries. 
If the path does not exist, print "No such file or directory", and return an 
ENOENT. */

unsigned char *disk;
struct ext2_group_desc *grpdsc;
struct ext2_inode *inode_table;

int next_inode(int inode_num, char *dir, struct ext2_inode *inode);

int main(int argc, char **argv) {

  if( (argc < 3 || argc > 4) || (argc == 4 && argv[2] != "-a")) {
    printf("AAA: %s/n BBB: %s/n", argv[1], argv[2]);
    fprintf(stderr, "Usage: readimg <image file name> <optional: flag -a> <absolute path>\n");
    exit(1);
    }

    int fd = open(argv[1], O_RDWR);
 //    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
 //  	if(disk == MAP_FAILED) {
 //  		perror("mmap");
 //    	exit(1);
	// }

    // call hlper function
	read_disk(fd);

	char *path = argv[2];
	char *absolute;
	// get the directory from the absolute path
	absolute = strtok(path, "/");


    // start from root (inode number 2)
    int inode_dir = 2;
    while (absolute != NULL) {

        inode_dir = find_dir(inode_dir-1, absolute);
        
        // No such directory is found
        if (inode_dir == -1) {
            printf("No such file or directory\n");
            return 0;
        }

        // next directory in absolute path
        absolute = strtok(NULL, "/");
    }


    // helper: print to user
    print_inode(inode_dir-1);

    return 0;
}