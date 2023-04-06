#include "syscall.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesys.h"
#include "paging.h"

// variable to know which is current process
uint32_t current_pid = 0;

// array of processes (to find available process number)
uint32_t pid_array[NUM_PROCESS_MAX];

fd_array fd_arrays[NUM_PROCESS_MAX];

// pointer to a table of file operations
// e.g. keyboard_ptr = {keyboard_open, keyboard_close, ...}


// do not halt process 0
int32_t sys_halt(uint8_t status) {
    return 0;
}

/*
* sys_execute
*   DESCRIPTION: Execute program based on input command
*   INPUTS: command - the command to execute
*   RETURN VALUE: 0 on success, -1 on failure
*   SIDE EFFECTS: context switch to new process
*/
int32_t sys_execute(const uint8_t* command) {
    uint32_t pid;
    // check for NULL input
    if (!command) {
        return -1;
    }
    directory_entry_t dentry;
    // search for command in file system
    if (read_dentry_by_name(command, &dentry) == -1) {
        return -1;  // not found
    }
    // check file type(regular file)
    if (dentry.file_type != 2) {
        return -1;
    }
    // check ELF magic constant at the start of file


    // assign process number based on pid_array
    pid = 0;    // temporary assign to 0

    // set up paging
    page_init(pid);

    // program loader
    // read from file, copy to virtual memory starting at 0x08048000
    
    // set PCB
    // initialize fd_arrays[pid](stdin, stdout, program file)
    // fd_arrays[pid].fd[0].operator_ptr = ptr to a table of functions;
    // fd_arrays[pid].fd[0].file_position = 0;
    // fd_arrays[pid].fd[0].inode = ?;
    // fd_arrays[pid].fd[0].flag = 0;   // not in use
    

    // put fd to the start of 8KB block(8MB - (pid + 1)*8KB)
    uint32_t pcb_start = 0x800000 - (pid + 1) * 0x2000;
    uint32_t n_bytes =  128;    // 8 * 16Bytes

    // store parent process id after fd

    // set esp0, ss0 in TTS

    // context switch
    // push context on stack

    // call iret


    return 0;
}

int32_t sys_read(int32_t fd, void* buf, int32_t nbytes) {
    return 0;
};

int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes)  {
    return 0;
};

int32_t sys_open(const uint8_t* filename) {
    return 0;
};

int32_t sys_close(int32_t fd) {
    return 0;
};

int32_t sys_getargs(uint8_t* buf, int32_t n_bytes) {
    return 0;
};

int32_t sys_vidmap(uint8_t** screen_start) {
    return 0;
};

int32_t sys_set_handler(int32_t signum, void* handler_address) {
    return 0;
};

int32_t sys_sigreturn(void) {
    return 0;
};
