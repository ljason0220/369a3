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

extern unsigned char *disk;
extern struct ext2_group_desc *grpdsc;
extern struct ext2_inode *inode_table;

//Takes a fd and reads fd into disk and initialize group_descriptor and inode_table
void read_disk(int fd) {
	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	//if mmap failed, exit, otherwise initialize
	if (disk == MAP_FAILED) {
		perror("mmap failed");
		exit(1);
	}

	grpdsc = (struct ext2_group_desc *)(disk + EXT2_BLOCK_SIZE * 2);
	inode_table = (struct ext2_inode *)(disk + EXT2_BLOCK_SIZE * grpdsc->bg_inode_table);
}

//Takes an inode_num that we will look in for dir
int find_dir(int inode_num, char *dir) {
	
}