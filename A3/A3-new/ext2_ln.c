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

/* ext2_ln: This program takes three command line arguments. Eirst is name of 
an ext2 formatted virtual disk. The other two are absolute paths on your 
ext2 formatted disk. The program should work like ln, creating a link from 
the first specified file to the second specified path. If the source file 
does not exist (ENOENT), if the link name already exists (EEXIST), or if 
either location refers to a directory (EISDIR), then your program should 
return the appropriate error. */

unsigned char *disk;
struct ext2_group_desc *grpdsc;
struct ext2_inode *inode_table;

int main(int argc, char **argv) {
	//Check proper number of arguments
	if(argc != 4) {
		fprintf(stderr, "ext2_ln <name of ext2 virtual disk> [-s <symlink flag>] <source file abs path> <dest location abs path>\n");
		exit(1);
	}
	//Initialize variables to supplied arguments
	int fd = open(argv[1], O_RDWR);
	char *src_path = argv[2];
	char *dst_path = argv[3];
	char src_name[80];
	strcpy(src_name, basename(src_path));
    char dst_name[80];
    strcpy(dst_name, basename(dst_path));

    //helper function
    //reads to grpdsc and inode_table
    read_disk(fd);

    //traverse to src_path and find inode_number using find_dir
    char *src_path_token;
    src_path_token = strtok(src_path, "/");
    //root inode is inode number 2
    int inode_dir = 2;
    while (src_path_token != NULL) {
    	//if token = src_name then we are in the right inode_dir, exit loop
    	if (strcmp(src_name, src_path_token) == 0) {
    		break;
    	}

    	//reach here then we haven't found the right inode_dir yet
    	inode_dir = find_dir(inode_dir-1, src_path_token);

    	//inode_dir we are looking for doesn't exist
    	if (inode_dir == -1) {
    		printf("No such file or directory in source path\n");
    		return ENOENT;
    	}

    	//we have not found it yet but find_dir didn't return error(-1)
    	//so advance to next token and look there
    	src_path_token = strtok(NULL, "/");
    }

    //last check for file in directory
    int src_inode = find_dir(inode_dir-1, src_name);
    if (src_inode == -1) {
    	printf("Source file does not exist\n");
    	return ENOENT;
    }

    //traverse to dst_path
    char *dst_path_token;
    dst_path_token = strtok(dst_path, "/");
    //root inode is inode number 2
    int inode_dir2 = 2;
    while (dst_path_token != NULL) {
    	//if token = dst_name then we are in the right inode_dir, exit loop
    	if (strcmp(dst_name, dst_path_token) == 0) {
    		break;
    	}

    	//reach here then we haven't found the right inode_dir2 yet
    	inode_dir2 = find_dir(inode_dir-1, dst_path_token);

    	//inode_dir2 we are looking for doesn't exist
    	if (inode_dir == -1) {
    		printf("No such file or directory in destination path\n");
    		return ENOENT;
    	}

    	//we have not found it yet but find_dir didn't return error(-1)
    	//so advance to next token and look there
    	dst_path_token = strtok(NULL, "/");
    }

    //last check for file in directory
    //find_dir should return a -1 as an error because it should find nothing
    int dst_inode = find_dir(inode_dir2-1, dst_name);
    if (dst_inode != -1) {
    	printf("Destination file already exists\n");
    	return EEXIST;
    }

    //helper function
    set_new_entry(inode_dir2-1, src_inode, dst_name, 1);

    return 0;
}