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

/* ext2_rm: This program takes two command line arguments. 1. name of an ext2 
formatted virtual disk; and 2. an absolute path to a file or link (not a 
directory) on that disk. The program should work like rm, removing the 
specified file from the disk. If the file does not exist or if it is a 
directory, then your program should return the appropriate error. */

unsigned char *disk;
struct ext2_group_desc *grpdsc;
struct ext2_inode *inode_table;


int main(int argc, char **argv) {

    //check input arguments
    if(argc != 3) {
        fprintf(stderr, "Usage: ext2_rm <image file name> <absolute path>\n");
        exit(1);
    }

    //process arguments
    int fd = open(argv[1], O_RDWR);
    char *abs_path = argv[2];
    char *rm_name = basename(abs_path);
    abs_path = strtok(abs_path, "/");

    //initiate using read_disk
    read_disk(fd);

    //traverse through directory
    int inode_dir = 2;
    while (abs_path != NULL) {

        inode_dir = find_dir(inode_dir-1, abs_path);
        
        // didn't find path 
        if (inode_dir == -1) {
            printf("Directory does not exist\n");
            return ENOENT;
        }

        // check path correctness
        if (strcmp(rm_name, abs_path) == 0) {
            break;
        }

        // next dir in absolute path
        abs_path = strtok(NULL, "/");
        printf("next %s\n", abs_path);
    }

    //Check file existence 
    int rm_inode = find_dir(inode_dir-1, rm_name);
    if (rm_inode == -1) {
        printf("File does not exist\n");
        return ENOENT;
    }

    //file found and remove
    remove_entry(inode_dir, rm_inode, rm_name);

    return 0;
}