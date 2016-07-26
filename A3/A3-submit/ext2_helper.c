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

extern unsigned char *disk;
extern struct ext2_group_desc *gd;
extern struct ext2_inode *inode_table;


/* 
INPUT: 	fd - file descriptor of a disk image

TASK: 	reads fd into disk and initialize 
*/
void read_disk(int fd) {
	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

	gd = (struct ext2_group_desc *)(disk + EXT2_BLOCK_SIZE * 2);
    inode_table = (struct ext2_inode *)(disk + EXT2_BLOCK_SIZE * gd->bg_inode_table);
}


/* 
INPUT: 	inode_num 	- the inode number we plan to look through
		dir 		- name of a directory

TASK:	looks through the inode (inode_num) and tries to find dir.
		returns the inode number for dir if exist, else -1
*/
int find_dir(int inode_num, char *dir) {
    struct ext2_inode       cur_inode = inode_table[inode_num];
    int                     block_idx;
    struct ext2_dir_entry_2 *dir_entry;   

    int block_read = 0;
    int next_inode = -1;

    int i;
    for (i = 0; i < 12; i++) {

    	block_idx = cur_inode.i_block[i];
    	if (block_idx == 0) return -1;
    	dir_entry = (struct ext2_dir_entry_2*)(disk + EXT2_BLOCK_SIZE * block_idx);

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


/* 
INPUT: 	inode_num 	- the inode number we plan to look through

TASK:	looks through the inode (inode_num) and prints its 
		directory entries by their name.
*/
void print_inode(int inode_num) {
	struct ext2_inode       cur_inode = inode_table[inode_num];
    int                     block_idx;
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


/*
TASK: 	sets the next available inode bit map and returns the
		corresponding index of it. otherwise return -1
*/
int set_inode_bitmap() {
    unsigned char *inodebitmap;
    inodebitmap = (disk + EXT2_BLOCK_SIZE * gd->bg_inode_bitmap);

    int index = 1;

    int i;
    for(i = 0;i < EXT2_BLOCK_SIZE/(sizeof(int) * 64); i++){
        int j;
        for(j = 7; j >= 0; j--){
            // check for unused inode in bitmap
            if ((!!((inodebitmap[i] << j) & 0x80)) == 0) {
                gd->bg_free_inodes_count--;
                inodebitmap[i] = inodebitmap[i] ^ (0x1 << (7-j));
                return index;
            }
            index++;
        }
    }

    return -1;
}

/*
TASK: 	sets the next available block bit map and returns the
		corresponding index of it. otherwise return -1
*/
int set_block_bitmap() {
	unsigned char *blockbitmap;
    blockbitmap = (disk + EXT2_BLOCK_SIZE * gd->bg_block_bitmap);

    int index = 1;

    int i;
    for(i = 0;i < EXT2_BLOCK_SIZE/(sizeof(int) * 16); i++){
        int j;
        for(j = 7; j >= 0; j--){
            // check for unused inode in bitmap
            if ((!!((blockbitmap[i] << j) & 0x80)) == 0) {
                gd->bg_free_blocks_count--;
                blockbitmap[i] = blockbitmap[i] ^ (0x1 << (7-j));
                return index;
            }
            index++;
        }
    }

	return -1;
}

/*
INPUT:	inode_num 	- the inode number that we wish to make and set its values
		source		- the data of a file we wish to copy into the data blocks	

TASK:	sets a new inode with blocks filled with data using source
*/
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

/* 
INPUT:	inode_num 		- the inode number we are looking in
		entry_inode_num	- the inode number we wish to set as an entry inside inode_num
		file_name		- the file name we wish to set with the entry

TASK:	Put a new file entry into the inode (inode_num). This new entry
		will point to entry_inode_num and have its name set to file_name.
*/
int set_new_entry(int inode_num, int entry_inode_num, char *file_name, int entry_type) {
    int                     block_idx;
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


/* 
INPUT:  inode_num       - the inode number we plan to set as a directory
        parent_inode    - the inode number with entry linking to inode_num

TASK:   Sets a new directory inode with contents of "." & ".."
*/
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

    gd->bg_used_dirs_count++;
}


/* 
INPUT:  parent_inode - the directory inode for the inode file to want to remove
        inode_num    - the inode we want to remove

TASK:   Remove the inode_num entry from parent_inode and update the inode inode_num
*/
void remove_entry(int parent_inode, int inode_num, char *rm_name) {
    int                     block_idx;
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