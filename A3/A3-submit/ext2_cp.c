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
    if(argc != 4) {
        fprintf(stderr, "Usage: ext2_cp <image file name> <path to file> <copy into absolute path> \n");
        exit(1);
    }

    /* READ FROM STANDARD INPUT */
    int fd = open(argv[1], O_RDWR);
    char *file_path = argv[2];
    FILE* file_to_copy = fopen(file_path, "r");
    char *dest_path = argv[3];
    dest_path = strtok(dest_path, "/");

    /* READ THE DISK & INITIALIZE */
    read_disk(fd);
    
    /* FILE DOES NOT EXIST */
    if(file_to_copy == NULL) {
        printf("file no exist\n");
        return ENOENT;
    } 

    /* START AT ROOT AND TRAVERSE */
    int inode_dir = 2;
    while (dest_path != NULL) {

        inode_dir = find_dir(inode_dir-1, dest_path);

        // we did not find it...
        if (inode_dir == -1) {
            printf("directory no exist\n");
            return ENOENT;
        }

        // next dir in path
        dest_path = strtok(NULL, "/");
    }

    /* CHECK IF FILE ALREADY EXIST IN DIRECTORY*/
    char *file_name = basename(argv[2]);
    int temp = find_dir(inode_dir-1, file_name);
    if (temp != -1) {
        printf("file already exist\n");
        return EEXIST;
    }
    
    /* FIND A AVAILABLE INODE */
    int new_inode_idx = set_inode_bitmap();

    /* INITIALIZE THAT INODE WITH FILE INTO ITS BLOCKS */
    set_new_inode(new_inode_idx - 1, file_to_copy);

    /* SET A NEW ENTRY (INODE WE JUST CREATED) IN THE DIRECTORY INODE */ 
    set_new_entry(inode_dir-1, new_inode_idx, file_name, 1);

    return 0;
}