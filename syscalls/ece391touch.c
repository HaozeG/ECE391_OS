#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main() {
    int32_t fd, cnt;
    uint8_t buf[1024];

    if (0 != ece391_getargs (buf, 1024)) { // get the filename
        ece391_fdputs (1, (uint8_t*)"could not read arguments\n");
	    return 3;
    }
    if (-1 == (fd = ece391_open ((uint8_t*)"."))) {
        ece391_fdputs (1, (uint8_t*)"directory open failed\n");
        return 2;
    }
    cnt = ece391_strlen(buf); // get the length of the filename
    if (-1 == ece391_write (fd, buf, cnt)){
        return 3;
    }
    return 0;
}