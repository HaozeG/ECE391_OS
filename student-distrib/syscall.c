#include "syscall.h"
#include "x86_desc.h"
#include "lib.h"

int32_t sys_halt(uint8_t status) {

    return 0;
}

int32_t sys_execute(const uint8_t* command) {


    return 0;
}
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t sys_open(const uint8_t* filename);
int32_t sys_close(int32_t fd);
int32_t sys_getargs(uint8_t* buf, int32_t n_bytes);
int32_t sys_vidmap(uint8_t** screen_start);
int32_t sys_set_handler(int32_t signum, void* handler_address);
int32_t sys_sigreturn(void);
