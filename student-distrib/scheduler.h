#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "lib.h"
#include "types.h"
#include "syscall.h"

#define NUM_ACTIVE_PROC NUM_TERM

pcb_t *run_queue[NUM_ACTIVE_PROC];

void schedule();

#endif
