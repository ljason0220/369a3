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

int next_inode(int inode_num, char *dir, struct ext2_inode *inode);

int main(int argc, char **argv) {

    /* ERROR CHECK NUMBER OF ARUGMENTS */
    if(argc != 3) {
        fprintf(stderr, "Usage: ext2_mkdir <image file name> <absolute path>\n");
        exit(1);
    }

    /* READ THE ARGUMENTS */
    int fd = open(argv[1], O_RDWR);
    char *abs_path = argv[2];
    char *dir_name = basename(argv[2]);
    abs_path = strtok(abs_path, "/");

    /* READ DISK AND INITIALIZE */
    read_disk(fd);

    /* START AT ROOT AND TRAVERSE */
    int inode_dir = 2;
    while (abs_path != NULL) {

        // we are now in the correct path
        if (strcmp(dir_name, abs_path) == 0) {
            break;
        }


        inode_dir = find_dir(inode_dir-1, abs_path);
        
        // we did not find it...
        if (inode_dir == -1) {
            printf("No such file or directory\n");
            return ENOENT;
        }

        // next dir in absolute path
        abs_path = strtok(NULL, "/");
    }


    /* CHECK IF FOLDER ALREADY EXIST IN DIRECTORY*/
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