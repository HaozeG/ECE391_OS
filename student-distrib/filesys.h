#ifndef FILESYS_H
#define FILESYS_H
#include "types.h"

#ifndef ASM

// need boot block, data block, inode, and directory entry
typedef struct directory_entry { // 64 bytes
    uint8_t file_name[32];
    uint32_t file_type;
    uint32_t inode_num;
    uint8_t reserved[24];
} directory_entry_t; 

typedef struct boot_block { // 4 KiB
    uint32_t num_dir_entries; 
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint8_t reserved[52];
    directory_entry_t dir_entries[63];
} boot_block_t;

typedef struct data_block { // 4 KiB
    uint8_t data[4096];
} data_block_t;

typedef struct index_node { // 4 KiB
    uint32_t length;
    uint32_t data_blocks[1023];
} inode_t;

typedef struct file { // temporary file descriptor structure for the read file and read directory functions. 
    uint32_t file_operations_table_pointer;
    uint32_t inode;
    uint32_t file_position;
    uint32_t flags;
} file_t;

boot_block_t* boot_block_ptr; // global pointers to important locations
data_block_t* data_start_ptr;
inode_t* inode_start_ptr;
directory_entry_t* direc_entry_start_ptr;

// all the read functions needed by the base file system and also the initialization function. 
int32_t read_dentry_by_name (const uint8_t* fname, directory_entry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, directory_entry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
void filesys_init(uint32_t filesys_addr);

// directory related functions
int32_t read_directory(file_t* fd, void* buf, int32_t nbytes);

int32_t open_directory(const uint8_t* filename);

int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes);

int32_t close_directory(int32_t fd);

// file related functions
int32_t read_file(file_t* fd, void* buf, int32_t nbytes);

int32_t open_file(const uint8_t* filename);

int32_t write_file(int32_t fd, const void* buf, int32_t nbytes);

int32_t close_file(int32_t fd);

#endif /* ASM */

#endif /* FILESYS_H */
