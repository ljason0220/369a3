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
    // keep track of block
    int count = 0;
    // init next inode
    int next = -1;

    //f firat 11 inodes are reserved
    int i;
    for (i = 0; i < 12; i++) {

    	block = cur_inode.i_block[i];
    	if (block == 0) return -1;
    	dir_entry = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE * block);

    	count = 0;
    	while(count < EXT2_BLOCK_SIZE) {

	        // matched
	        if (strncmp(dir, dir_entry->name, dir_entry->name_len) == 0) {
	            next = dir_entry->inode;
	            return next;
	        }

	        // increment & check
	        count += dir_entry->rec_len;

	        // the next link
	        dir_entry = (void *)dir_entry + dir_entry->rec_len;
	    }
    }

    return next;
}


void print_inode(int inode_num) {
	struct ext2_inode cur_inode = inode_table[inode_num];
    int block_idx;
    struct ext2_dir_entry_2 *dir_entry;   

    int block_read = 0;
    int i;
    for (i = 0; i < 12; i++) {

    	block_idx = cur_inode.i_block[i];
    	if (block_idx == 0) return;

    	dir_entry = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE * block_idx);

    	block_read = 0;
    	while(block_read < EXT2_BLOCK_SIZE) {

        	printf("%.*s\n", dir_entry->name_len, dir_entry->name);

	        // increment & check
	        block_read += dir_entry->rec_len;

	        // the next link
	        dir_entry = (void *)dir_entry + dir_entry->rec_len;
	    }
    }
}


int set_block_bitmap() {
	unsigned char *blockbitmap;
    blockbitmap = (disk + EXT2_BLOCK_SIZE * grpdsc->bg_block_bitmap);

    int index = 1;

    int i;
    for(i = 0;i < EXT2_BLOCK_SIZE/(sizeof(int) * 16); i++){
        int j;
        for(j = 7; j >= 0; j--){
            // check for unused inode in bitmap
            if ((!!((blockbitmap[i] << j) & 0x80)) == 0) {
                grpdsc->bg_free_blocks_count--;
                blockbitmap[i] = blockbitmap[i] ^ (0x1 << (7-j));
                return index;
            }
            index++;
        }
    }

	return -1;
}


int set_inode_bitmap() {
    unsigned char *inodebitmap;
    inodebitmap = (disk + EXT2_BLOCK_SIZE * grpdsc->bg_inode_bitmap);

    int index = 1;

    int i;
    for(i = 0;i < EXT2_BLOCK_SIZE/(sizeof(int) * 64); i++){
        int j;
        for(j = 7; j >= 0; j--){
            // check for unused inode in bitmap
            if ((!!((inodebitmap[i] << j) & 0x80)) == 0) {
                grpdsc->bg_free_inodes_count--;
                inodebitmap[i] = inodebitmap[i] ^ (0x1 << (7-j));
                return index;
            }
            index++;
        }
    }

    return -1;
}


void set_new_inode(int inode_num, FILE *source) {

    inode_table[inode_num].i_mode = EXT2_S_IFREG;
    inode_table[inode_num].i_links_count = 1;

    int total_bytes = 0;
    int i;

    // write into the first 12 blocks
    for (i = 0; i < 12; i++) {
    	// use available block slot
    	inode_table[inode_num].i_block[i] = set_block_bitmap();
    	void *block = (void *)(disk + EXT2_BLOCK_SIZE * inode_table[inode_num].i_block[i]);

    	// only read size of block 
        size_t bytes_read = fread(block, 1, EXT2_BLOCK_SIZE, source);
        total_bytes += bytes_read;

        if (bytes_read < EXT2_BLOCK_SIZE) {
        	inode_table[inode_num].i_size = total_bytes;
        	inode_table[inode_num].i_blocks = (i + 1) * 2;
	        return;
        }
    }

    // write into the 13 block (indirect ptr)
    inode_table[inode_num].i_block[12] = set_block_bitmap();
    unsigned int *block = (void *)(disk + EXT2_BLOCK_SIZE * inode_table[inode_num].i_block[12]);
    for (i = 0; i < EXT2_BLOCK_SIZE / (sizeof (int)); i++) {
        block[i] = set_block_bitmap();
        void *block_inside = (void *)(disk + EXT2_BLOCK_SIZE * block[i]);

        // only read size of block 
        size_t bytes_read = fread(block_inside, 1, EXT2_BLOCK_SIZE, source);
        total_bytes += bytes_read;

        if (bytes_read < EXT2_BLOCK_SIZE) {
            inode_table[inode_num].i_size = total_bytes;
            inode_table[inode_num].i_blocks = (i + 13) * 2;
            return;
        }

    } 
}


int set_new_entry(int inode_num, int entry_inode_num, char *file_name, int entry_type) {
    int block_idx;
    struct ext2_dir_entry_2 *dir_entry;    

    int block_read;
    int i, remainder, needed;

    needed = sizeof(struct ext2_dir_entry_2) + strlen(file_name) + 4;

    for (i = 0; i < 12; i++) {

    	block_idx = inode_table[inode_num].i_block[i];
    	dir_entry = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE * block_idx);

    	block_read = 0;
    	while(block_read < EXT2_BLOCK_SIZE) {

    		// this is the last entry
    		if ((block_read + dir_entry->rec_len) == 1024) {

				// calculate the true rec_len
				int temp_rec_len = sizeof(struct ext2_dir_entry_2) + dir_entry->name_len;
				while (temp_rec_len % 4 != 0) {
				    temp_rec_len++;
				}

				// enough room in block for new entry?
				remainder = (dir_entry->rec_len - temp_rec_len);
				if (remainder > needed) {
					dir_entry->rec_len = temp_rec_len;
					dir_entry = (void *)dir_entry + temp_rec_len;

					// init the values
					dir_entry->inode = entry_inode_num;
					dir_entry->rec_len = remainder;
					strcpy(dir_entry->name, file_name);
					dir_entry->name_len = strlen(strtok(file_name, "."));
					dir_entry->file_type = entry_type;
					return entry_inode_num;
				}

    		}

    		// increment
        	block_read += dir_entry->rec_len;

	        // the next link
	        dir_entry = (void *)dir_entry + dir_entry->rec_len;
    	}
    }

    return -1;
}


void set_dir_inode(int inode_num, int parent_inode) {
	inode_table[inode_num].i_block[0] = set_block_bitmap();
	struct ext2_dir_entry_2 *dir_entry = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE * inode_table[inode_num].i_block[0]);

    /* SET INODE DATA */
    inode_table[inode_num].i_mode = EXT2_S_IFDIR;
    inode_table[inode_num].i_links_count = 1;
    inode_table[inode_num].i_size = EXT2_BLOCK_SIZE;
    inode_table[inode_num].i_blocks = 2;

    /* SET ENTRY FOR "." */
    dir_entry->inode = inode_num + 1;
    dir_entry->rec_len = 12;
    dir_entry->name_len = 1;
    dir_entry->file_type = 2;
    strcpy(dir_entry->name, ".");

   	dir_entry = (void *)dir_entry + dir_entry->rec_len;

   	/* SET ENTRY FOR ".." */
    dir_entry->inode = parent_inode;
    dir_entry->rec_len = EXT2_BLOCK_SIZE - 12;
    dir_entry->name_len = 2;
    dir_entry->file_type = 2;
    strcpy(dir_entry->name, "..");

    grpdsc->bg_used_dirs_count++;
}


void remove_entry(int parent_inode, int inode_num, char *rm_name) {
    int block_idx;
    struct ext2_dir_entry_2 *dir_entry;    
    struct ext2_dir_entry_2 *dir_entry_ahead;    

    int block_read;
    int i;
    for (i = 0; i < 12; i++) {

        block_idx = inode_table[parent_inode-1].i_block[i];
        if (block_idx == 0) return;
        dir_entry = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE * block_idx);
        dir_entry_ahead = (void *)dir_entry + dir_entry->rec_len;

        block_read = 0;
        while(block_read < EXT2_BLOCK_SIZE) {

            // this is the entry we want to remove
            if (strncmp(rm_name, dir_entry_ahead->name, dir_entry_ahead->name_len) == 0) {
                
                // just add rec_len of dir_entry to pass this entry
                dir_entry->rec_len = dir_entry->rec_len + dir_entry_ahead->rec_len;
                return;
            }

            // increment
            block_read += dir_entry->rec_len;

            // the next link
            dir_entry = (void *)dir_entry + dir_entry->rec_len;
            dir_entry_ahead = (void *)dir_entry_ahead + dir_entry_ahead->rec_len;
        }
    }
}	