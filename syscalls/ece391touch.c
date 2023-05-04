#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main() {
    int32_t fd, cnt;
    uint8_t buf[1024];
    uint8_t buf_[64001];

    if (0 != ece391_getargs (buf, 1024)) { // get the filename
        ece391_fdputs (1, (uint8_t*)"could not read arguments\n");
	    return 3;
    }
    if (-1 == (fd = ece391_open ((uint8_t*)"."))) {
        ece391_fdputs (1, (uint8_t*)"directory open failed\n");
        return 2;
    }
    cnt = ece391_strlen(buf); // get the length of the filename
    // buf[cnt] = '\0';
    // cnt++;
    if (-1 == ece391_write (fd, buf, cnt)){
        return 3;
    }
    ece391_close(fd);
    // TODO: Here I have a loop to assign value for buf
    // And manually change cnt
    int i;
    for (i = 0; i < 64000; i++) {
        buf_[i] = '2';
    }
    buf_[i] = 0;
    cnt = 64000;

    if (-1 == (fd = ece391_open ((uint8_t*)buf))) {
        ece391_fdputs (1, (uint8_t*)"file open failed\n");
        return 2;
    }
    cnt = ece391_strlen(buf_); // get the length of the filename
    cnt = 10000;
    buf[cnt] = '\0';
    cnt++;

    if (-1 == ece391_write (fd, buf_, cnt)){
        return 3;
    }

    return 0;
}
