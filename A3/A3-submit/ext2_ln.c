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
    if(argc != 4) {
        fprintf(stderr, "Usage: ext2_ln <image file name> <source link path> <dest link path>\n");
        exit(1);
    }

    /* READ THE ARGUMENTS */
    int fd = open(argv[1], O_RDWR);
    char *source_path = argv[2];
    char *dest_path = argv[3];
    char source_name[50];
    strcpy(source_name, basename(source_path));
    char dest_name[50];
    strcpy(dest_name, basename(dest_path));


    /* READ DISK AND INITIALIZE */
    read_disk(fd);

    /* START AT ROOT AND TRAVERSE FOR SOURCE*/
    source_path = strtok(source_path, "/");
    int inode_dir = 2;
    while (source_path != NULL) {

        // we are now in the correct path
        if (strcmp(source_name, source_path) == 0) {
            break;
        }

        inode_dir = find_dir(inode_dir-1, source_path);
        
        // we did not find it...
        if (inode_dir == -1) {
            printf("No such file or directory in source path\n");
            return ENOENT;
        }

        // next dir in absolute path
        source_path = strtok(NULL, "/");
    }


    /* CHECK IF SOURCE EXIST IN THE DIRECTORY*/
    int source_inode = find_dir(inode_dir-1, source_name);
    if (source_inode == -1) {
        printf("source file does not exist\n");
        return ENOENT;
    }


    /* START AT ROOT AND TRAVERSE FOR DESTINATION */
    dest_path = strtok(dest_path, "/");
    int inode_dir2 = 2;
    while (dest_path != NULL) {

        // we are now in the correct path
        if (strcmp(dest_name, dest_path) == 0) {
            break;
        }


        inode_dir2 = find_dir(inode_dir2-1, dest_path);
        
        // we did not find it...
        if (inode_dir2 == -1) {
            printf("No such file or directory in dest path\n");
            return ENOENT;
        }

        // next dir in absolute path
        dest_path = strtok(NULL, "/");
    }

    /* CHECK IF DEST ALREADY EXIST IN THE DIRECTORY*/
    int temp = find_dir(inode_dir2-1, dest_name);
    if (temp != -1) {
        printf("destination file already exist\n");
        return EEXIST;
    }


    /* SET THE NEW ENTRY AND POINT TO SOURCE INODE */
    set_new_entry(inode_dir2-1, source_inode, dest_name, 1);

    return 0;
}