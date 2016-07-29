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
	// current inode
    struct ext2_inode cur_inode = inode_table[inode_num];
    // block index
    int block;
    // directory entry
    struct ext2_dir_entry_2 *dir_entry;   

    int block_read = 0;
    int next_inode = -1;

    int i;
    for (i = 0; i < 12; i++) {

    	block = cur_inode.i_block[i];
    	if (block == 0) return -1;
    	dir_entry = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE * block);

    	block_read = 0;
    	while(block_read < EXT2_BLOCK_SIZE) {

	        // we found it!
	        //printf("compare: %s vs %s\n", dir, dir_entry->name);
	        if (strncmp(dir, dir_entry->name, dir_entry->name_len) == 0) {
	            next_inode = dir_entry->inode;
	            return next_inode;
	        }

	        // increment & check
	        block_read += dir_entry->rec_len;

	        // the next link
	        dir_entry = (void *)dir_entry + dir_entry->rec_len;
	    }
    }

    return next_inode;

}