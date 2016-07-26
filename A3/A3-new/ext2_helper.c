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
extern struct ext2_group_desc *gd;
extern struct ext2_inode *inode_table;

void read_disk(int fd) {

}

int find_dir(int inode_num, char *dir) {
	
}