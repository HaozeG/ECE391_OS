#include "scheduler.h"
#include "terminal.h"
#include "x86_desc.h"

/*
* schedule
*   DESCRIPTION: Implement scheduler driven by PIT in Round-robin way
*   INPUTS: none
*   RETURN VALUE: none
*   SIDE EFFECTS: change running_term and running process
*/
void schedule() {
    cli();
    // find the next one in the run_queue
    int32_t new_pid = current_pid;
    pcb_t *next_pcb = get_pcb_ptr(current_pid);
    int i;
    for (i = 1; i <= NUM_ACTIVE_PROC; i++) {
        if (run_queue[(running_term + i) % NUM_ACTIVE_PROC] != 0) {
            next_pcb = run_queue[(running_term + i) % NUM_ACTIVE_PROC];
            new_pid = next_pcb->pid;
            break;
        }
    }
    pcb_t *prev_pcb = get_pcb_ptr(current_pid);

    // TODO: check memcopy(what condition to copy/restore; which one to copy/restore)
    if (running_term == display_term) {
        // store current video memory
        memcpy((void *)(VID_MEM_TERM0 + running_term * fourKB), (void *)VID_MEM_ADDR, fourKB);
    }

    page_init(new_pid);
    flush_tlb();

    // set esp0, ss0 in TTS
    tss.esp0 = (uint32_t)(0x800000 - new_pid * 0x2000 - 4);     // end of 8KB block(4 for return address)
    tss.ss0 = KERNEL_DS;

    // save ebp, esp of current process
    register uint32_t saved_ebp asm("ebp");
    register uint32_t saved_esp asm("esp");
    prev_pcb->saved_ebp = saved_ebp;
    prev_pcb->saved_esp = saved_esp;
    current_pid = new_pid;

    running_term = next_pcb->terminal;
    vmem_remap();
    if (running_term == display_term) {
        // restore new video memory
        memcpy((void *)VID_MEM_ADDR, (void *)(VID_MEM_TERM0 + display_term * fourKB), fourKB);
    } 
    // context switch
    asm volatile ("                     \n\
            movl %0, %%ebp              \n\
            movl %1, %%esp              \n\
            "
            :                         \
            :  "r"(next_pcb->saved_ebp), "r"(next_pcb->saved_esp)\
            :  "memory"
    );
    sti();
}
