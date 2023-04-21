#include "syscall.h"
#include "scheduler.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesys.h"
#include "paging.h"
#include "keyboard.h"
#include "terminal.h"
#include "rtc.h"

// variable to know which is current process
uint32_t current_pid = 1;
// variable to know if exception occured
uint8_t exp_occured = 0;
uint8_t is_base_shell = 1;

// array of processes (to find available process number)
uint8_t pid_array[NUM_PROCESS_MAX];

// First four bytes at the start of executable file
uint8_t magic_numbers[4] = {0x7F, 0x45, 0x4C, 0x46};

// pointer to a table of file operations
static uint32_t *stdin_table[] = {(uint32_t *)terminal_open, (uint32_t *)terminal_close, (uint32_t *)terminal_read, (uint32_t *)illegal};
static uint32_t *stdout_table[] = {(uint32_t *)terminal_open, (uint32_t *)terminal_close, (uint32_t *)illegal, (uint32_t *)terminal_write};
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
int32_t sys_halt(uint8_t status) {
    cli();
    // do not halt base shell of each terminal
    if (current_pid == (running_term + 1) || current_pid == 0) {
        printf("--RESTART BASE SHELL--\n");
        // restart shell
        pid_array[current_pid] = 0;
        is_base_shell = 1;
        run_queue[running_term] = 0;
        sti();
        sys_execute((uint8_t *)"shell \n");
        // should never reach here
        return SYSCALL_FAIL;
    }

    pcb_t *current_pcb = get_pcb_ptr(current_pid);
    uint32_t parent_pid = current_pcb->parent_pid;

    // clear PCB
    current_pcb->parent_pid = 0;
    current_pcb->saved_ebp = 0;
    current_pcb->saved_esp = 0;
    current_pcb->saved_eip = 0;
    current_pcb->terminal = 0;
    // clear fd
    int i;
    for (i = 0; i < 8; i++) {
        sys_close(i);
    }

    // clear value in pid_array
    pid_array[current_pid] = 0;

    // set esp0, ss0 in TTS
    tss.esp0 = (uint32_t)(0x800000 - parent_pid * 0x2000 - 4);     // end of 8KB block(4 for return address)
    tss.ss0 = KERNEL_DS;
    // set up paging
    page_init(parent_pid);
    flush_tlb();
    // undo vidmap of previous process
    process_paging[current_pid].page_directory[(USER_ADDR_VIRTUAL + fourMB) >> 22].pde_page_table.present = 0;
    run_queue[running_term] = get_pcb_ptr(parent_pid);

    current_pid = parent_pid;
    vmem_remap();
    current_pcb = get_pcb_ptr(current_pid);
    sti();
    // return to execute
    asm volatile ("                     \n\
            andl $0, %%eax             \n\
            movb %1, %%al               \n\
            movb %4, %%ah               \n\
            movb $0, %0                 \n\
            movl %2, %%esp              \n\
            movl %3, %%ebp              \n\
            jmp RET_FROM_HALT                         \n\
            "
            : "=g"(exp_occured)                          \
            : "r"(status), "r"(current_pcb->saved_esp), "r"(current_pcb->saved_ebp), "r"(exp_occured)\
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
    uint32_t new_pid = 0;
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
    pid_array[0] = 1;       // "kernel process", never return to pid 0
    cli();
    if (is_base_shell) {
        new_pid = running_term + 1;
        is_base_shell = 0;
    } else {
        i = 0;
        // pid = 1, 2, 3 for base shell of terminal 0, 1, 2
        for (i = NUM_TERM + 1; i < NUM_PROCESS_MAX; i++) {
            if (pid_array[i] == 0) {
                pid_array[i] = 1;
                new_pid = i;
                break;
            }
        }
        if (i == NUM_PROCESS_MAX || i == 0) {
            printf("CANNOT CREATE MORE PROCESS!\n");
            sti();
            return SYSCALL_FAIL;
        }
    }
    // set up paging
    page_init(new_pid);
    flush_tlb();

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

    register uint32_t saved_ebp asm("ebp");
    register uint32_t saved_esp asm("esp");
    pcb_t *current_pcb = get_pcb_ptr(current_pid);
    current_pcb->saved_ebp = saved_ebp;
    current_pcb->saved_esp = saved_esp;

    // set PCB
    // initialize fd(stdin, stdout, program file)
    current_pcb = get_pcb_ptr(new_pid);
    current_pcb->fd[0].flags = 1;
    current_pcb->fd[0].file_position = 0;
    current_pcb->fd[0].inode = 0;
    current_pcb->fd[0].file_operations_table_pointer = (fot_t*)stdin_table;
    current_pcb->fd[1].flags = 1;
    current_pcb->fd[1].file_position = 0;
    current_pcb->fd[1].inode = 0;
    current_pcb->fd[1].file_operations_table_pointer = (fot_t*)stdout_table;
    // store parent process id
    current_pcb->parent_pid = current_pid;
    current_pcb->pid = new_pid;
    current_pid = new_pid;

    // set esp0, ss0 in TTS
    tss.esp0 = (uint32_t)(0x800000 - new_pid * 0x2000 - 4);     // end of 8KB block(4 for return address)
    tss.ss0 = KERNEL_DS;

    // Parse arguments
    int32_t arg_len = (int32_t)strlen ((int8_t *)arg);
    strncpy ((int8_t *)current_pcb->args, (int8_t *)arg, (uint32_t)arg_len);
    current_pcb->args[arg_len] = 0;
    current_pcb->saved_eip = prog_eip;
    current_pcb->terminal = running_term;
    run_queue[running_term] = current_pcb;
    vmem_remap();

    sti();
    // context switch
    // push context on stack
    // eip = prog_eip
    // esp = 132MB - 4B = 0x083FFFFC
    // 0x2B: USER_DS
    // TODO: sti() by changing new EFLAGS
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
    pcb_t* pcb_ptr = get_pcb_ptr(current_pid);
    if (fd < 0 || fd > 7 || pcb_ptr->fd[fd].flags == 0 || buf == 0 || nbytes < 0) {
        return SYSCALL_FAIL;
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
    pcb_t* pcb_ptr = get_pcb_ptr(current_pid);
    if (fd < 0 || fd > 7 || pcb_ptr->fd[fd].flags == 0 || buf == 0 || nbytes < 0) {
        return SYSCALL_FAIL;
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
    pcb_t* pcb_ptr = get_pcb_ptr(current_pid);
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
            // call actual open function of the device
            pcb_ptr->fd[i].file_operations_table_pointer->open(filename);
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
    pcb_t* pcb_ptr = get_pcb_ptr(current_pid);
    if (pcb_ptr->fd[fd].flags == 0) {
        return -1; // cannot close unopened file
    }
    pcb_ptr->fd[fd].flags = 0;
    pcb_ptr->fd[fd].file_position = 0; // might not be needed since when you open a file, you init these values.
    pcb_ptr->fd[fd].inode = 0;
    return pcb_ptr->fd[fd].file_operations_table_pointer->close(fd);
};

/*
* sys_getargs
*   DESCRIPTION: read command line arguments into a user-level buffer
*   INPUTS: buf - pointer to user level buffer
*           n_bytes - maximum bytes to read
*   RETURN VALUE: 0 on success, -1 on failure
*   SIDE EFFECTS:
*/
int32_t sys_getargs(uint8_t* buf, int32_t n_bytes) {
    if (n_bytes <= 0 || buf == NULL) {
        return SYSCALL_FAIL;
    }
    pcb_t *current_pcb = get_pcb_ptr(current_pid);
    if (current_pcb->args[0] == '\0')
    {
        return SYSCALL_FAIL;
    }
    else
    {
        strncpy((int8_t *)buf, (int8_t *)current_pcb->args, n_bytes);
        return 0;
    }
};

/*
* sys_vidmap
*   DESCRIPTION: Map video memory to virtual address 132MB, assign address to the address pointed by input pointer
*   INPUTS: screen_start - pointer to the pointer of video memory
*   RETURN VALUE: 0 on success, -1 on failure
*   SIDE EFFECTS: Add a new 4KB page at 132MB
*/
int32_t sys_vidmap(uint8_t** screen_start) {
    if(screen_start == NULL || (uint32_t)screen_start < USER_ADDR_VIRTUAL || (uint32_t)screen_start > (USER_ADDR_VIRTUAL + fourMB - 1)) {
        return SYSCALL_FAIL;; //check if screen is within bounds
    }
    cli();
    vmem_remap();
    // store virtual video memory address as a pointer in user space
    *screen_start = (uint8_t*)(USER_ADDR_VIRTUAL + fourMB);
    sti();
    return 0;
};

int32_t sys_set_handler(int32_t signum, void* handler_address) {
    if (signum < 0 || signum >= NUM_VEC || handler_address == 0) {
        return SYSCALL_FAIL;
    }
    return SYSCALL_FAIL;
};

int32_t sys_sigreturn(void) {
    return SYSCALL_FAIL;
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
pcb_t* get_pcb_ptr(uint32_t pid) {
    return (pcb_t *)(0x00800000 - (pid + 1) * 0x2000);
}

// function for illegal file operation
int32_t illegal(int32_t fd, void* buf, int32_t nbytes) {
    return -1;
}

/*
* flush_tlb
*   DESCRIPTION: Flush TLB by writing to cr3
*   INPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: TLB flushed
*/
void flush_tlb() {
    // TLB flush
    asm volatile ("                     \n\
            movl %%cr3, %%eax           \n\
            movl %%eax, %%cr3           \n\
            "
            :                           \
            :                           \
            :  "eax"
    );
}

/*
* vmem_remap
*   DESCRIPTION: Remap virtual memory relating to video memory
*   INPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: Change virtual video memory based on current_pid
*/
void vmem_remap() {
    // set up page directory entry to 4KiB page
    process_paging[current_pid].page_directory[(USER_ADDR_VIRTUAL + fourMB) >> 22].pde_page_table.present = 1;
    pcb_t *current_pcb = get_pcb_ptr(current_pid);
    if (current_pcb->terminal == display_term) {
        // map to displaying vmem
        // for user
        process_paging[current_pid].pte_vidmap[0].page_base_addr = VID_MEM_ADDR >> 12;   //video memory address
        // for kernel
        process_paging[current_pid].page_table[VID_MEM_ADDR >> 12].page_base_addr = VID_MEM_ADDR >> 12;
    } else {
        // map to non-displaying vmem
        // for user
        process_paging[current_pid].pte_vidmap[0].page_base_addr = (VID_MEM_TERM0 + running_term * fourKB) >> 12;   //video memory address
        // for kernel
        process_paging[current_pid].page_table[VID_MEM_ADDR >> 12].page_base_addr = (VID_MEM_TERM0 + running_term * fourKB) >> 12;
    }
    flush_tlb();
}
