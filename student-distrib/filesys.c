#include "filesys.h"
#include "lib.h"

void filesys_init(uint32_t* filesys_addr) {
    boot_block_ptr = (boot_block_t*)filesys_addr;
    data_start_ptr = (data_block_t*)(filesys_addr + (boot_block_ptr->num_inodes + 1)*4096);
    inode_start_ptr = (inode_t*)(filesys_addr + 4096);
    direc_entry_start_ptr = boot_block_ptr->dir_entries;
}

int32_t read_dentry_by_name (const uint8_t* fname, directory_entry_t* dentry) {
    int i;
    int len = strlen((int8_t*)fname);
    if (fname == 0 || dentry == 0) { // NULL check
        return -1;
    }
    for (i = 0; i < boot_block_ptr->num_dir_entries; i++) {
        if (strncmp((int8_t*)fname, (int8_t*)boot_block_ptr->dir_entries[i].file_name, len) == 0) {
            read_dentry_by_index(i, dentry);
            return 0;
        }
    }
    return -1; 
}

int32_t read_dentry_by_index (uint32_t index, directory_entry_t* dentry) {
    if (index  < 0 || index > 62 || index >= boot_block_ptr->num_dir_entries) { // might be a redundant check. 
        return -1; 
    }
    strncpy((int8_t*)dentry->file_name, (int8_t*)boot_block_ptr->dir_entries[index].file_name, 32);
    dentry->file_type = boot_block_ptr->dir_entries[index].file_type;
    dentry->inode_num = boot_block_ptr->dir_entries[index].inode_num;
    return 0;
}

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    int i;
    int data_block_index;
    int byteIndex; 
    int bytes_left_to_read = length; 
    int buf_idx = 0;
    int bytes_read = 0;
    inode_t* inode_b = (inode_t*)(boot_block_ptr + 1 + inode);
    int length_of_file = inode_b->length;
    if (inode + 1 > boot_block_ptr->num_inodes) {
        return -1; 
    }
    if (offset > length_of_file) {
        return 0; 
    }

    data_block_index = offset / 4096; 
    byteIndex = offset % 4096;

    for (i = byteIndex; i < 4096; i++) {
        if (bytes_left_to_read == 0) {
            break; 
        } else {
            buf[buf_idx] = inode_b->data_blocks[data_block_index].data[i];
            bytes_left_to_read--; 
            bytes_read++;
            buf_idx++;
        }
    }
    while (bytes_left_to_read != 0) {
        data_block_index++;
        for (i = 0; i < 4096; i++) {
            if (bytes_left_to_read == 0) {
                break;
            } else {
                buf[buf_idx] = inode_b->data_blocks[data_block_index].data[i];
                bytes_left_to_read--;
                bytes_read++;
                buf_idx++;
            }
        }
    }
    return bytes_read;
}
// input : a pointer to an fd entry
// buffer to store the res
// number of bytes to read
int32_t read_directory(file_t* fd, void* buf, int32_t nbytes) { // write all file names into the buf
    //uint32_t inum = fd->inode;
    if (buf == 0) {
        return -1;
    }
    directory_entry_t* dentry; 
    if (read_dentry_by_index(fd->file_position, dentry) == -1) {
        return 0;
    }
    fd->file_position++;
    uint32_t bytes_read = strlen((int8_t*)dentry->file_name);
    strncpy(buf, (int8_t*)dentry->file_name, 32);
    return bytes_read;
}

int32_t open_directory(const uint8_t* filename) {
    return 0;
}

int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

int32_t close_directory(int32_t fd) {
    return 0;
}


int32_t read_file(file_t* fd, void* buf, int32_t nbytes) {
    if (buf == 0) {
        return -1;
    }
    uint32_t bRead = read_data(fd->inode, fd->file_position, buf, nbytes);
    return bRead;
}

int32_t open_file(const uint8_t* filename) {

    return 0;
}

int32_t write_file(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

int32_t close_file(int32_t fd) {
    return 0;
}


