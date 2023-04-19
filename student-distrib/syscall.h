#ifndef SYSCALL_H_
#define SYSCALL_H_
#include "lib.h"
#include "filesys.h"
#include "types.h"
#include "paging.h"

#define SYSCALL_FAIL -1
// System call numbers
#define SYS_HALT    1
#define SYS_EXECUTE 2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETARGS 7
#define SYS_VIDMAP  8
#define SYS_SET_HANDLER  9
#define SYS_SIGRETURN  10

extern uint8_t exp_occured;
// all system calls supported
extern int32_t sys_halt(uint8_t status);
extern int32_t sys_execute(const uint8_t* command);
extern int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t sys_open(const uint8_t* filename);
extern int32_t sys_close(int32_t fd);
extern int32_t sys_getargs(uint8_t* buf, int32_t n_bytes);
extern int32_t sys_vidmap(uint8_t** screen_start);
extern int32_t sys_set_handler(int32_t signum, void* handler_address);
extern int32_t sys_sigreturn(void);
extern uint32_t current_pid;

int32_t program_loader(uint32_t inode);

typedef struct file_operation_table {
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
} fot_t;

typedef struct file_descriptor {
    fot_t* file_operations_table_pointer;
    uint32_t inode;
    uint32_t file_position;
    uint32_t flags;
} fd_t;

// PCB - start of 8KB
typedef struct {
    fd_t fd[8];
    uint32_t pid;
    uint32_t parent_pid;
    uint32_t saved_esp;
    uint32_t saved_ebp;
    uint32_t saved_eip;
    uint8_t args[128];
} pcb_t;

int32_t illegal(int32_t fd, void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t close(int32_t fd);
pcb_t* get_pcb_ptr();
void flush_tlb();

extern uint32_t current_pid;
extern uint8_t pid_array[NUM_PROCESS_MAX];
extern pcb_t pcb_array[NUM_PROCESS_MAX];

#endif
