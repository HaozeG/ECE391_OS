#include "filesys.h"
#include "lib.h"
#include "syscall.h"

/*
* filesys_init
*   DESCRIPTION: initializes the global pointers, maps the correct locations of
*                boot block, inode start, and also data block start to our file system. 
*   INPUTS: filesys_addr - the starting address of the boot block. 
*   RETURN VALUE: none
*   SIDE EFFECTS: our file system is mapped to the img.
*/
data_block_map_t dblock_map; 
void filesys_init(uint32_t filesys_addr) {
    int i, j, tmp; 
    boot_block_ptr = (boot_block_t*)filesys_addr; // initialize all the global pointers
    inode_start_ptr = (inode_t*)(boot_block_ptr+1);
    data_start_ptr = (data_block_t*)(inode_start_ptr+boot_block_ptr->num_inodes);
    direc_entry_start_ptr = boot_block_ptr->dir_entries;
    for (i = 0; i < 1024; i++) {
        dblock_map.in_use[i] = 0;
    }
    dblock_map.num_datablocks_active = 0;
    for (i = 0; i < boot_block_ptr->num_inodes; i++) {
        tmp = (inode_start_ptr+i)->length / 4096 + 1; // this is how many datablocks is being used by the inode;
        for (j = 0; j < tmp; j++) {
            dblock_map.in_use[(inode_start_ptr+i)->data_blocks[j]] = 1; // fill in which data blocks are already being used by the file system.
        }
        dblock_map.num_datablocks_active += tmp; 
    }
}

/*
* read_dentry_by_name
*   DESCRIPTION: reads a directory entry by the name you provide to it and stores its data into a directory entry
*                structure that you provide to it. 
*   INPUTS: fname - the file name as a character array
*           dentry - the directory entry structure that will be written into. 
*   RETURN VALUE: 0 for success, -1 for failed to read, or failed to find a file with that name. 
*   SIDE EFFECTS: calls read_dentry_by_index to actually write into the dentry. 
*/
int32_t read_dentry_by_name (const uint8_t* fname, directory_entry_t* dentry) {
    int i;
    int len = strlen((int8_t*)fname);
    if (len > 32) { // if the length of the file name is longer than the max, it cannot exist in our filesystem.
        return -1; 
    }
    if (fname == 0 || dentry == 0) { // NULL check
        return -1;
    }
    for (i = 0; i < boot_block_ptr->num_dir_entries; i++) {
        if (strncmp((int8_t*)fname, (int8_t*)boot_block_ptr->dir_entries[i].file_name, 32) == 0) { // if the bytes match, then provide index and read by index
            read_dentry_by_index(i, dentry);
            return 0;
        }
    }
    return -1; 
}

/*
* read_dentry_by_name
*   DESCRIPTION: reads a directory entry by the index found in read_dentry_by_index and writes its contents from
*                the directory entry into the dentry struct that you provide.
*   INPUTS: index - the index of the directory entry in perspective of boot_block. 
*           dentry - the directory entry structure that will be written into. 
*   RETURN VALUE: 0 for success, -1 for failed to read, or failed to find a file with that name. 
*   SIDE EFFECTS: writes the contents of a directory entry in our file system into the directory entry structure we provide. 
*/
int32_t read_dentry_by_index (uint32_t index, directory_entry_t* dentry) {
    if (dentry == 0 || index  < 0 || index > 62 || index >= boot_block_ptr->num_dir_entries) { // might be a redundant check. 
        return -1; 
    }
    strncpy((int8_t*)dentry->file_name, (int8_t*)boot_block_ptr->dir_entries[index].file_name, 32); // copy the contents of the name into the dentry
    dentry->file_type = boot_block_ptr->dir_entries[index].file_type; // copy the rest of the fields into the dentry. 
    dentry->inode_num = boot_block_ptr->dir_entries[index].inode_num;
    return 0;
}

/*
* read_data
*   DESCRIPTION: reads data from a file starting from position located at offset and up to the length provided
*                then writes it into buf. 
*   INPUTS: inode - the inode number corresponding to the file we want to read.
*           offset - which byte inside the file we should start to read from. 
*           buf - the data buffer that will hold the data read.
*           length - the amount of bytes to read from the file. 
*   RETURN VALUE: returns the number of bytes read. Special cases are: -1 is error, inode doesn't exist, etc. 0 is failed to read any bytes. 
*   SIDE EFFECTS: will write into buf with data read from the file disignated by inode. 
*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    int i; // iterator
    int data_block_index; // which data_block we're at in the file. 
    int byteIndex; // which byte we are at in the file.
    int bytes_left_to_read = length; // how many bytes do we have to read.
    int buf_idx = 0; // index into our buffer
    int bytes_read = 0; // the amount of bytes that we have copied/read.
    inode_t* inode_b = inode_start_ptr+inode; // the pointer to the inode to our file. 
    int length_of_file = inode_b->length; 
    // uint32_t bnum; // the data block number. 
    // printf("length of file: %d \n", length_of_file);
    if (inode + 1 > boot_block_ptr->num_inodes) { // is the inode valid check.
        return -1; 
    }
    if (offset > length_of_file) { // if we start at more than the number of bytes inside the file, fail to read anything.
        return 0; 
    }
    if (length_of_file < length) { // if the length of the file is shorter than the amount we want to read, then we set the bytes we need to read to the length of the file. 
        bytes_left_to_read = length_of_file;
    }
    if (offset + length > length_of_file) { // if we start at some position and we will read more bytes than the file can provide, provide the correct bound.
        bytes_left_to_read = length_of_file - offset;
    }
    data_block_index = offset / 4096; // starting data block provided by the inode.
    byteIndex = offset % 4096; // starting index of the byte inside the file. 

    for (i = byteIndex; i < 4096; i++) { // this loop ensures that we always start to read from byteIndex 0 in the next loop. 
        if (bytes_left_to_read == 0) { // done condition
            break; 
        } else { // copy data into buffer and increment/decrement counters.
            buf[buf_idx] = (data_start_ptr+(inode_b->data_blocks[data_block_index]))->data[i];
            bytes_left_to_read--; 
            bytes_read++;
            buf_idx++;
        }
    }
    while (bytes_left_to_read != 0) { // this loop will keep reading until we're done. 
        data_block_index++;
        for (i = 0; i < 4096; i++) {
            if (bytes_left_to_read == 0) {
                break;
            } else {
                buf[buf_idx] = (data_start_ptr+(inode_b->data_blocks[data_block_index]))->data[i];
                bytes_left_to_read--;
                bytes_read++;
                buf_idx++;
            }
        }
    }
    return bytes_read;
}
// returns the number of bytes successfully written to the file. 
int32_t write_data(uint32_t inode, uint8_t* buf, uint32_t length) {
    if (inode >= boot_block_ptr->num_inodes || buf == NULL) { // is the inode valid check.
        return 0; 
    }
    int i;
    int bytes_written = 0;
    inode_t* inode_b = inode_start_ptr+inode; // the pointer to the inode to our file. 
    int len = inode_b->length;
    int data_block_index = len/4096; 
    int start_index = len % 4096;
    int buf_idx = 0;
    if (start_index == 0) {
        for (i = 0; i < 1024; i++) {
            if (dblock_map.in_use[i] == 0) {
                inode_b->data_blocks[data_block_index] = i;
            }
        }
    }

    for (i = start_index; i < 4096; i++) { // this loop ensures that we always start to read from byteIndex 0 in the next loop. 
        if (bytes_written == length) { // done condition
            break; 
        } else { // copy data into buffer and increment/decrement counters.
            (data_start_ptr+(inode_b->data_blocks[data_block_index]))->data[i] = buf[buf_idx]; 
            bytes_written++;
            buf_idx++;
            inode_b->length++;
        }
    }
    while (bytes_written != length) { // execute until all bytes are written to the file. 
        // check if the current most data block has space available. 
        // if we hit the end of this data block, check the map for the next available one and keep writing
        data_block_index++;
        for (i = 0; i < 1024; i++) {
            if (dblock_map.in_use[i] == 0) {
                inode_b->data_blocks[data_block_index] = i; // find the next available data block. 
            }
        }
        for (i = 0; i < 4096; i++) {
            if (bytes_written == length) {
                break;
            } else {
                (data_start_ptr+(inode_b->data_blocks[data_block_index]))->data[i] = buf[buf_idx];
                bytes_written++;
                buf_idx++;
                inode_b->length++;
            }
        }
    }
    return bytes_written; 
}
/*
* read_directory
*   DESCRIPTION: reads a directory (copies name of a file inside the directory into a buffer)
*   INPUTS: fd - file descriptor proxy. 
*           buf - buffer to write the filename into. 
*           nbytes - number of bytes to be read, but seems useless
*   RETURN VALUE: nbytes for success, 0 for failed to read, or failed to find a file with that name. 
*   SIDE EFFECTS: writes the name of a file inside our directory into the buffer.
*/
int32_t read_directory(int32_t fd, void* buf, int32_t nbytes) { // write all file names into the buf
    //uint32_t inum = fd->inode;
    if (buf == 0) {
        return 0;
    }
    directory_entry_t dentry; 
    int8_t *buf_to = (int8_t *)buf;
    
    pcb_t* pcb_ptr = (pcb_t *)(0x00800000 - (current_pid + 1) * 0x2000);
    if (read_dentry_by_index(pcb_ptr->fd[fd].file_position, &dentry) < 0) {
        return 0;
    }
    uint32_t bytes_read = (uint32_t)strcpy(buf_to, (int8_t *)dentry.file_name, 32);
    pcb_ptr->fd[fd].file_position++;
    
    return bytes_read;
}

/*
* open_directory
*   DESCRIPTION: opens a directory (sets up its file descriptor inside the array)
*   INPUTS: filename - name of the directory. 
*   RETURN VALUE: 0 for success, -1 for failed to read, or failed to find a file with that name. 
*   SIDE EFFECTS: "opens" a directory and sets up 
*/
int32_t open_directory(const uint8_t* filename) {
    return 0;
}

/*
* write_directory
*  DOES NOTHING WE ARE READ ONLY FILE SYSTEM. ** EC IS WRITEABLE FILE SYSTEM**
*  THIS FUNCTION WILL NOW CREATE A NEW FILE IN THE FILESYSTEM, VIA 'TOUCH' COMMAND IN TERMINAL
*  buf is the file name 
*/
int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes) {
    directory_entry_t* dentry;
    if (fd < 2 || fd > 7) {
        return -1; 
    }
    int i; 
    int8_t* name = (int8_t*) buf; 
    if (name == NULL || strlen(name) > 32) { // check if name is valid, i.e. not null and not more than 32 characters.
        return -1; 
    }
    if (boot_block_ptr->num_inodes > 63) { // no more space `
        return -1; 
    }
    for (i = 1; i < 63; i++) {
        if (read_dentry_by_name((direc_entry_start_ptr+i)->file_name, dentry) == -1) { // we found a dentry that's not in use. 
            strncpy((int8_t*)(direc_entry_start_ptr+i)->file_name, name, 32);
            (direc_entry_start_ptr+i)->file_type = 2; // we are only able to write regular files. 
            (direc_entry_start_ptr+i)->inode_num = boot_block_ptr->num_inodes; // allocate the next inode since we have no holes in our inodes.
            boot_block_ptr->num_inodes++; 
            return 0;
        }
    }
    return -1; 
}

/*
* read_directory
*   DESCRIPTION: closes the directory, I think it will remove the file descriptor from the array.
*   INPUTS: fd - file descriptor proxy. 
*   RETURN VALUE: 0 for success, -1 for failed to read, or failed to find a file with that name. 
*   SIDE EFFECTS: closes the directory. 
*/
int32_t close_directory(int32_t fd) {
    return 0;
}

/*
* read_file
*   DESCRIPTION: reads the contents of a file 
*   INPUTS: fd - file descriptor proxy. 
*           buf - buffer to write the filename into. 
*           nbytes - number of bytes to be read, but seems useless
*   RETURN VALUE: -1 for failed, and number of bytes read is returned. 
*   SIDE EFFECTS: buf is written into. 
*/
int32_t read_file(int32_t fd, void* buf, int32_t nbytes) {
    if (buf == 0) {
        return -1;
    }
    pcb_t* pcb_ptr = (pcb_t *)(0x00800000 - (current_pid + 1) * 0x2000);
    uint32_t bRead = read_data(pcb_ptr->fd[fd].inode, pcb_ptr->fd[fd].file_position, buf, nbytes);
    if (bRead < 0) { // read failed.
        return -1;
    }
    pcb_ptr->fd[fd].file_position += bRead; // increment file position to designate where to start next read.
    return bRead;
}

/*
* open_file
*   DESCRIPTION: opens a file
*   INPUTS: filename - name of the file
*   RETURN VALUE: 0 for success, -1 for failed to read, or failed to find a file with that name. 
*   SIDE EFFECTS: I guess it is supposed to set up the file descriptor inside the file descriptor array. 
*/
int32_t open_file(const uint8_t* filename) {

    return 0;
}

/*
* write_file
*   USELESS WE ARE A READ ONLY FILE SYSTEM.
*/
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/*
* close_file
*   DESCRIPTION: closes a file
*   INPUTS: fd - file descriptor proxy. 
*   RETURN VALUE: 0 for success, -1 for failed to read, or failed to find a file with that name. 
*   SIDE EFFECTS: probably removes the file descriptor off of the file descriptor array. 
*/
int32_t close_file(int32_t fd) {
    return 0;
}


