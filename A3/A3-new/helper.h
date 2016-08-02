// define functions for helper_functions.c

extern unsigned char *disk;
extern struct ext2_group_desc *grpdsc;
extern struct ext2_inode *inode_table;
void read_disk(int fd);
int find_dir(int inode_num, char *dir);
void print_inode(int inode_num);
int set_inode_bitmap();
int set_block_bitmap();
void set_new_inode(int inode_num, FILE *source);
int set_new_entry(int inode_num, int entry_inode_num, char *file_name, int entry_type);
void set_dir_inode(int inode_num, int parent_inode);
void remove_entry(int parent_inode, int inode_num, char *rm_name);