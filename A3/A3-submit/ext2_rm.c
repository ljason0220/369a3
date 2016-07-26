#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include <string.h>
#include <errno.h>
#include <libgen.h>

unsigned char *disk;
struct ext2_group_desc *gd;
struct ext2_inode *inode_table;


int main(int argc, char **argv) {

    /* ERROR CHECK NUMBER OF ARUGMENTS */
    if(argc != 3) {
        fprintf(stderr, "Usage: ext2_rm <image file name> <absolute path>\n");
        exit(1);
    }

    /* READ THE ARGUMENTS */
    int fd = open(argv[1], O_RDWR);
    char *abs_path = argv[2];
    char *rm_name = basename(abs_path);
    abs_path = strtok(abs_path, "/");

    /* READ DISK AND INITIALIZE */
    read_disk(fd);

    /* START AT ROOT AND TRAVERSE */
    int inode_dir = 2;
    while (abs_path != NULL) {

        // we are now in the correct path
        if (strcmp(rm_name, abs_path) == 0) {
            break;
        }

        inode_dir = find_dir(inode_dir-1, abs_path);
        
        // we did not find it...
        if (inode_dir == -1) {
            printf("Directory does not exist\n");
            return ENOENT;
        }
        // next dir in absolute path
        abs_path = strtok(NULL, "/");
        printf("next %s\n", abs_path);
    }

    /* CHECK IF FILE TO BE REMOVE NO DIRECTORY */
    int rm_inode = find_dir(inode_dir-1, rm_name);
    if (rm_inode == -1) {
        printf("File does not exist\n");
        return ENOENT;
    }

    /* REMOVE THE ENTRY */
    remove_entry(inode_dir, rm_inode, rm_name);

    return 0;
}

