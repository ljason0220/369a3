#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"
#include <string.h>

unsigned char *disk;
struct ext2_group_desc *gd;
struct ext2_inode *inode_table;

int next_inode(int inode_num, char *dir, struct ext2_inode *inode);

int main(int argc, char **argv) {

    /* ERROR CHECK NUMBER OF ARUGMENTS */
    if(argc != 3) {
        fprintf(stderr, "Usage: ext2_ls <image file name> <absolute path>\n");
        exit(1);
    }

    /* READ THE ARGUMENTS */
    int fd = open(argv[1], O_RDWR);
    char *abs_path = argv[2];
    abs_path = strtok(abs_path, "/");

    /* READ DISK AND INITIALIZE */
    read_disk(fd);

    /* START AT ROOT AND TRAVERSE */
    int inode_dir = 2;
    while (abs_path != NULL) {

        inode_dir = find_dir(inode_dir-1, abs_path);
        
        // we did not find it...
        if (inode_dir == -1) {
            printf("No such file or directory\n");
            return 0;
        }

        // next dir in absolute path
        abs_path = strtok(NULL, "/");
    }


    /* READ/PRINT THE INODE THAT WE FOUND */
    print_inode(inode_dir-1);

    return 0;
}