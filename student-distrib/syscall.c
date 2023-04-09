#include "syscall.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesys.h"
#include "paging.h"
#include "keyboard.h"
#include "terminal.h"
#include "rtc.h"

// variable to know which is current process
uint32_t current_pid = 1;

// array of processes (to find available process number)
static uint8_t pid_array[NUM_PROCESS_MAX];

static pcb_t pcb_array[NUM_PROCESS_MAX];

// First four bytes at the start of executable file
uint8_t magic_numbers[4] = {0x7F, 0x45, 0x4C, 0x46};

// pointer to a table of file operations
static uint32_t *std_table[] = {(uint32_t *)terminal_open, (uint32_t *)terminal_close, (uint32_t *)terminal_read, (uint32_t *)terminal_write};
static uint32_t *file_table[] = {(uint32_t *)open_file, (uint32_t *)close_file, (uint32_t *)read_file, (uint32_t *)write_file};
static uint32_t *dir_table[] = {(uint32_t *)open_directory, (uint32_t *)close_directory, (uint32_t *)read_directory, (uint32_t *)write_directory};
static uint32_t *rtc_table[] = {(uint32_t *)rtc_open, (uint32_t *)rtc_close, (uint32_t *)rtc_read, (uint32_t *)rtc_write};

/*
* sys_halt
*   DESCRIPTION: halt current process and return to execute
*   INPUTS: status - return value for execute system call
*   RETURN VALUE: none
*   SIDE EFFECTS: Go to execute but not caller
*/
int32_t sys_halt(uint16_t status) {
    // do not halt process 0 or 1
    if (current_pid == 0 || current_pid == 1) {
        printf("--RESTART BASE SHELL--\n");
        // restart shell
        pid_array[current_pid] = 0;
        sys_execute((uint8_t *)"shell \n");
        // should never reach here
        return SYSCALL_FAIL;
    }
    
    uint32_t parent_pid = pcb_array[current_pid].parent_pid;

    // clear PCB
    pcb_array[current_pid].parent_pid = 0;
    pcb_array[current_pid].saved_ebp = 0;
    pcb_array[current_pid].saved_esp = 0;
    // clear fd
    int i;
    for (i = 0; i < 8; i++) {
        sys_close(i);
    }

    // clear value in pid_array
    pid_array[current_pid] = 0;
    // refresh pcb at the start of 8KB block(8MB - (pid + 1)*8KB)
    pcb_t* pcb_start = (pcb_t *)(0x00800000 - (current_pid + 1) * 0x2000);
    *pcb_start = pcb_array[current_pid];

    // set esp0, ss0 in TTS
    tss.esp0 = (uint32_t)(0x800000 - parent_pid * 0x2000 - 4);     // end of 8KB block(4 for return address)
    tss.ss0 = KERNEL_DS;
    // set up paging
    page_init(parent_pid);
    // TLB flush
    asm volatile ("                     \n\
            movl %%cr3, %%eax           \n\
            movl %%eax, %%cr3           \n\
            "
            :                           \
            :                           \
            :  "eax"
    );

    current_pid = parent_pid;
    // return to execute
    asm volatile ("                     \n\
            andl $0, %%eax             \n\
            movw %0, %%ax               \n\
            movl %1, %%esp              \n\
            movl %2, %%ebp              \n\
            jmp RET_FROM_HALT                         \n\
            "
            :                           \
            : "r"(status), "r"(pcb_array[parent_pid].saved_esp), "r"(pcb_array[parent_pid].saved_ebp)\
            :  "memory", "eax"
    );
    // never goes here
    return 0;
}

/*
* sys_execute
*   DESCRIPTION: Execute program based on input command
*   INPUTS: command - the command to execute
*   RETURN VALUE: 0-255 - execute a halt, 
*                   -1 - command cannot be executed
*                   256 - dies by exception
*   SIDE EFFECTS: context switch to new process
*/
int32_t sys_execute(const uint8_t* command) {
    uint32_t new_pid;
    int32_t prog_eip;
    int32_t return_val;
    int32_t i;
    uint8_t cmd[128],arg[128];

    // check for NULL input
    if (!command) {
        return SYSCALL_FAIL;
    }
    // Parse arguments
    int32_t command_len = (int32_t)strlen ((int8_t *)command);
    for (i = 0; i < command_len; i++)
    {
        if (command[i] == ' ')
          {
            strncpy ((int8_t *)cmd, (int8_t *)command, i);
            cmd[i] = '\0'; 
            strncpy ((int8_t *)arg, (int8_t *)(command + i + 1), (uint32_t)(command_len - i - 1));
            arg[command_len - i - 1] = '\0'; 
            break;
          }
    }
    if (i==command_len)
    {
        strncpy ((int8_t *)cmd, (int8_t *)command, i);
        cmd[i] = '\0'; 
        arg[0] = '\0';
    }

    directory_entry_t dentry;
    // search for command in file system
    if (read_dentry_by_name(cmd, &dentry) != 0) {
        return SYSCALL_FAIL;  // not found
    }
    // check file type(2 for regular file)
    if (2 != dentry.file_type) {
        return SYSCALL_FAIL;
    }
    // check 4bytes ELF magic constant at the start of file
    uint8_t elf_buf[4];
    read_data(dentry.inode_num, 0, elf_buf, 4);
    if (0 != strncmp((int8_t *)elf_buf, (int8_t *)magic_numbers, 4)) {
        return SYSCALL_FAIL;
    }

    // assign process number based on pid_array
    i = 0;
    pid_array[0] = 1;       // "kernel process", never return to pid 0
    // pid = 1 for base shell
    for (i = 1; i < NUM_PROCESS_MAX; i++) {
        if (pid_array[i] == 0) {
            pid_array[i] = 1;
            new_pid = i;
            break;
        }
    }
    if (i == NUM_PROCESS_MAX || i == 0) {
        return SYSCALL_FAIL;
    }

    cli();
    // set up paging
    page_init(new_pid);
    // TLB flush
    asm volatile ("                     \n\
            movl %%cr3, %%eax             \n\
            movl %%eax, %%cr3             \n\
            "
            :                           \
            :                           \
            :  "eax"
    );

    // load program image
    prog_eip = program_loader(dentry.inode_num);
    if (prog_eip == -1) {
        // revert paging
        page_init(current_pid);
        // clear pid in pid array
        pid_array[new_pid] = 0;
        sti();
        return SYSCALL_FAIL;
    }

    // set PCB
    // initialize fd(stdin, stdout, program file)
    pcb_array[new_pid].fd[0].flags = 1;
    pcb_array[new_pid].fd[0].file_position = 0;
    pcb_array[new_pid].fd[0].inode = 0;
    pcb_array[new_pid].fd[0].file_operations_table_pointer = (fot_t*)std_table;
    pcb_array[new_pid].fd[1].flags = 1;
    pcb_array[new_pid].fd[1].file_position = 0;
    pcb_array[new_pid].fd[1].inode = 0;
    pcb_array[new_pid].fd[1].file_operations_table_pointer = (fot_t*)std_table;
    // store parent process id
    pcb_array[new_pid].parent_pid = current_pid;
    // put pointer to pcb to the start of 8KB block(8MB - (pid + 1)*8KB)
    pcb_t* pcb_start = (pcb_t *)(0x00800000 - (new_pid + 1) * 0x2000);
    *pcb_start = pcb_array[new_pid];

    // set esp0, ss0 in TTS
    tss.esp0 = (uint32_t)(0x800000 - new_pid * 0x2000 - 4);     // end of 8KB block(4 for return address)
    tss.ss0 = KERNEL_DS;

    register uint32_t saved_ebp asm("ebp");
    register uint32_t saved_esp asm("esp");
    pcb_array[current_pid].saved_ebp = saved_ebp;
    pcb_array[current_pid].saved_esp = saved_esp;
    current_pid = new_pid;

    // Parse arguments
    int32_t arg_len = (int32_t)strlen ((int8_t *)arg);
    strncpy ((int8_t *)pcb_array[current_pid].args, (int8_t *)arg, (uint32_t)arg_len);
    pcb_array[current_pid].args[arg_len] = 0;

    sti();
    // context switch
    // push context on stack
    // eip = prog_eip
    // esp = 132MB - 4B = 0x083FFFFC
    // 0x2B: USER_DS
    asm volatile ("                     \n\
            movw $0x2B, %%ax            \n\
            movw %%ax, %%ds             \n\
            movw %%ax, %%es             \n\
            movw %%ax, %%fs             \n\
            movw %%ax, %%gs             \n\
            movl $0x083FFFFC, %%ebp     \n\
            pushl %1                    \n\
            pushl $0x083FFFFC           \n\
            pushfl                      \n\
            pushl %2                    \n\
            pushl %3                    \n\
            iret                        \n\
            RET_FROM_HALT:              \n\
            movl %%eax, %0               \n\
            "
            : "=r"(return_val)          \
            :  "b"(USER_DS), "c"(USER_CS), "d"(prog_eip)\
            :  "memory", "cc", "eax"
    );

    // halt return to here
    return return_val;
}

/*
* sys_read
*   DESCRIPTION: perform read based on fd
*   INPUTS: fd - file descriptor
*           buf - buffer to read to
*           nbytes - maximum bytes to read
*   RETURN VALUE: number of bytes read on success, -1 on failure
*   SIDE EFFECTS: 
*/
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes) {
    pcb_t* pcb_ptr = get_pcb_ptr();
    if (fd < 0 || fd > 7 || pcb_ptr->fd[fd].flags == 0 || buf == 0 || nbytes < 0) {
        return -1;
    }
    return pcb_ptr->fd[fd].file_operations_table_pointer->read(fd, buf, nbytes);
};

/*
* sys_write
*   DESCRIPTION: write from buffer
*   INPUTS: fd
*           buf - buffer to write from
*           nbytes - maximum bytes to write
*   RETURN VALUE: bytes written on success, -1 on failure
*   SIDE EFFECTS: 
*/
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes)  {
    pcb_t* pcb_ptr = get_pcb_ptr();
    if (fd < 0 || fd > 7 || pcb_ptr->fd[fd].flags == 0 || buf == 0 || nbytes < 0) {
        return -1;
    }
    return pcb_ptr->fd[fd].file_operations_table_pointer->write(fd, buf, nbytes);
};

/*
* sys_open
*   DESCRIPTION: open a file for use
*   INPUTS: filename
*   RETURN VALUE: file descriptor on success, -1 on failure
*   SIDE EFFECTS: 
*/
int32_t sys_open(const uint8_t* filename) {
    directory_entry_t dentry;
    int i;
    if (filename == 0 || read_dentry_by_name(filename, &dentry) == -1) {
        return -1;
    }
    pcb_t* pcb_ptr = get_pcb_ptr();
    for (i = 2; i < 8; i++) {
        if(pcb_ptr->fd[i].flags == 0) {
            pcb_ptr->fd[i].flags = 1; // busy
            pcb_ptr->fd[i].inode = dentry.inode_num;
            pcb_ptr->fd[i].file_position = 0;

            if(dentry.file_type == 0) { // rtc
                pcb_ptr->fd[i].file_operations_table_pointer = (fot_t*)(rtc_table);
            } else if (dentry.file_type == 1) { // directory
                pcb_ptr->fd[i].file_operations_table_pointer = (fot_t*)(dir_table);
            } else if (dentry.file_type == 2) { // regular file
                pcb_ptr->fd[i].file_operations_table_pointer = (fot_t*)(file_table);
            }

            return i; // return the file descriptor upon successful initialization
        }
    }
    return -1; // no open slot in the fd array
};

/*
* sys_close
*   DESCRIPTION: close a file
*   INPUTS: fd
*   RETURN VALUE: 0 on success, -1 on failure
*   SIDE EFFECTS: 
*/
int32_t sys_close(int32_t fd) {
    if (fd < 2 || fd > 7) {
        return -1;
    }
    pcb_t* pcb_ptr = get_pcb_ptr();
    if (pcb_ptr->fd[fd].flags == 0) {
        return -1; // cannot close unopened file
    }
    pcb_ptr->fd[fd].flags = 0;
    pcb_ptr->fd[fd].file_position = 0; // might not be needed since when you open a file, you init these values.
    pcb_ptr->fd[fd].inode = 0;
    return pcb_ptr->fd[fd].file_operations_table_pointer->close(fd);
};

int32_t sys_getargs(uint8_t* buf, int32_t n_bytes) {
    if (n_bytes <= 0) {
        return SYSCALL_FAIL;
    }
    if (pcb_array[current_pid].args[0] == '\0')
    {
        return SYSCALL_FAIL;
    }
    else
    {
        strncpy((int8_t *)buf, (int8_t *)pcb_array[current_pid].args, n_bytes);   
        return 0;
    }
};

int32_t sys_vidmap(uint8_t** screen_start) {
    return 0;
};

int32_t sys_set_handler(int32_t signum, void* handler_address) {
    if (signum < 0 || signum >= NUM_VEC || handler_address == 0) {
        return -1;
    }
    return 0;
};

int32_t sys_sigreturn(void) {
    return 0;
};

/*
* program_loader
*   DESCRIPTION: read from file inode, copy to virtual memory starting at 0x08048000
*   INPUTS: inode - inode of program file
*   RETURN VALUE: address of entry point on success, -1 on failure
*   SIDE EFFECTS: only copy up to ~4MB
*/
int32_t program_loader(uint32_t inode) {
    int32_t program_entry;
    inode_t *program_inode;
    if (inode >= boot_block_ptr->num_inodes) {
        return -1;
    }
    // read out program entry
    read_data(inode, 24, (uint8_t *)&program_entry, 4);

    program_inode = (inode_t *)(boot_block_ptr + 1 + inode);
    read_data(inode, 0, (uint8_t *)0x08048000, program_inode->length);    // starting addr 0x08048000
    return program_entry;
}

// use mask to get pcb pointer of current process
pcb_t* get_pcb_ptr() {
    // go to head of 8KB block
    uint32_t mask = 0xFFFFE000;
    uint32_t current_esp;
    asm volatile ("     \n\
        movl %%esp, %0  \n\
    "                   \
    : "=r"(current_esp) \
    :                   \
    : "memory"          \
    );
    return (pcb_t *)(mask & current_esp);
}
