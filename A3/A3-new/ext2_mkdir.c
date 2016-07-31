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

/* ext2_mkdir: This program takes two command line arguments. 1. name of an 
ext2 formatted virtual disk; and 2. an absolute path on your ext2 formatted 
disk. The program should work like mkdir, creating the final directory on 
the specified path on the disk. If any component on the path to the location 
where the final directory is to be created does not exist or if the specified 
directory already exists, then your program should return the appropriate 
error (ENOENT or EEXIST). */

unsigned char *disk;
struct ext2_group_desc *grpdsc;
struct ext2_inode *inode_table;

int next_inode(int inode_num, char *dir, struct ext2_inode *inode);

int main(int argc, char **argv) {
    if(argc != 3) {
        fprintf(stderr, "Usage: ext2_mkdir <image file name> <absolute path>\n");
        exit(1);
    }

    int fd = open(argv[1], O_RDWR);\
    read_disk(fd);

    char *absolute = argv[2];
    char *dir_name = basename(argv[2]);
    absolute = strtok(absolute, "/");

    //start from root
    int inode_dir = 2;
    while (absolute != NULL) {

    	// we are now in the correct path
        if (strcmp(dir_name, absolute) == 0) {
            break;
        }


        inode_dir = find_dir(inode_dir-1, absolute);
        
        // error check: DNE
        if (inode_dir == -1) {
            printf("No such file or directory\n");
            return ENOENT;
        }

        // next dir in absolute path
        absolute = strtok(NULL, "/");
    }


    // error check: already exist
    int temp = find_dir(inode_dir-1, dir_name);
    if (temp != -1) {
        printf("folder already exist\n");
        return EEXIST;
    }


    /* FIND A AVAILABLE INODE */
    int new_inode_idx = set_inode_bitmap();

    /* SET THIS NEW INODE AS A DIRECTORY*/
    set_dir_inode(new_inode_idx - 1, inode_dir);

    /* INSERT THE NEW INODE AS A DIRECTORY ENTRY */
    set_new_entry(inode_dir - 1, new_inode_idx, dir_name, 2);

    return 0;

}