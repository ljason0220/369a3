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

int main(int argc, char **argv) {
	if(argc != 4) {
    	fprintf(stderr, "Usage: ext2_cp <image file name> <path of source> <absolute path of target>\n");
    	exit(1);
    }

    int fd =open(argv[1], O_RDWR);
    read_disk(fd);

    char *path_of_source = argv[2];
    FILE* file_to_copy = fopen(path_of_source, "r");
    char *dest_path = argv[3];
    dest_path = strtok(dest_path, "/");


    // like in ls, we start from root
    int inode_dir = 2;
    while (dest_path != NULL) {

        inode_dir = find_dir(inode_dir-1, dest_path);

        // if directory not find
        if (inode_dir == -1) {
            printf("directory no exist\n");
            return ENOENT;
        }

        // go to the next directory
        dest_path = strtok(NULL, "/");
    }

    // check error, if file already exists
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