#include "terminal.h"
#include "lib.h"
#include "keyboard.h"
#include "paging.h"
#include "syscall.h"
#include "x86_desc.h"
#include "scheduler.h"

int running_term = 0;    // changed by scheduler        [0, NUM_TERM - 1]
int display_term = 0;    // changed by terminal_switch  [0, NUM_TERM - 1]
// --- Terminal ---

int32_t terminal_open()
{
    clear();
    // Clears buf
    count_char[display_term] = 0;
    // int32_t i;
    // for (i = 0; i < MAX_BUF; i++) {
    //     terminal_buf[i] = 0;
    // }

    return 0;
}

int32_t terminal_close(int32_t fd)
{
    return -1;
}

/*
 * terminal_read
 *   DESCRIPTION: Read from the keyboard buffer into buf. Return if enter is pressed
 *   INPUTS: fd - file descriptor
 *           buf - the buffer that is copied into
 *           n_bytes - maximum bytes to copy
 *   RETURN VALUE: number of bytes copied(include \n) on success,
 *                 -1 on failure
 *   SIDE EFFECTS: Only returns until enter is pressed;
 */
int32_t terminal_read(int32_t fd, void *buf, int32_t n_bytes)
{
    int32_t i;
    char *buf_to = (char *)buf;
    // if (!buf_to || n_bytes > MAX_BUF || n_bytes < 0)
    if (!buf_to || n_bytes < 0)
    {
        return -1;
    }
    // output existing contents in keyboard buffer to screen
    kbd_buffer[display_term][count_char[display_term]] = '\0';
    puts((int8_t *)kbd_buffer[display_term]);
    cli();
    // Wait until enter pressed
    while (!enter_buf)
    {
        sti();
        // create delay
        for (i = 0; i < 200; i++)
        {
        }
        cli();
    };

    i = 0;
    // MAX_BUF: up to 128 characters
    while (i < MAX_BUF && i < count_char[display_term] && i < n_bytes)
    {
        buf_to[i] = kbd_buffer[display_term][i];
        i++;
    }
    buf_to[MAX_BUF - 1] = '\n';

    // reset enter indicator
    enter_buf = 0;
    count_char[display_term] = 0;
    sti();
    return i;
}

/*
 * terminal_write
 *   DESCRIPTION: Write from buf to screen.
 *   INPUTS: fd - file descriptor
 *           buf - the buffer that is write from
 *           n_bytes - number of bytes to write
 *   RETURN VALUE: number of bytes written(include '\n') on success, -1 on failure
 *   SIDE EFFECTS: Change content on screen
 */
int32_t terminal_write(int32_t fd, const void *buf, int32_t n_bytes)
{
    int32_t i;
    char *buf_from = (char *)buf;
    // if (!buf_from || n_bytes > MAX_BUF || n_bytes < 0)
    if (!buf_from || n_bytes < 0)
    {
        return -1;
    }

    i = 0;
    cli();
    // if (buf_from[i] == '\n') {
    //     putc('\n');
    // }
    // while (i < MAX_BUF && i < n_bytes && buf_from[i] != '\n')
    // while (i < MAX_BUF && i < n_bytes)
    while (i < n_bytes)
    {
        putc(buf_from[i]);
        i++;
        // if (buf_from[i] == '\n') {
        //     putc('\n');
        // }
    }

    sti();
    return i;
}

/*
 * terminal_switch
 *   DESCRIPTION: Switch to new terminal
 *   INPUTS: new_term_index - index of new terminal [0, 2]
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Change content on screen
 */
int32_t terminal_switch(int new_term_index) {
    if (new_term_index < 0 || new_term_index > 2) {
        return -1;
    }
    // clear func key buffer require holding
    ctrl_buf = 0;
    alt_buf = 0;
    if (running_term == display_term) {
        // store current video memory
        memcpy((void *)(VID_MEM_TERM0 + display_term * fourKB), (void *)VID_MEM_ADDR, fourKB);
    }

    // switch process to the active one of the new terminal
    uint32_t new_pid = 0;
    // retrieve process number of new terminal
    if (run_queue[(int)new_term_index] == 0) {
        display_term = new_term_index;
        running_term = new_term_index;
        vmem_remap();
        // restore new video memory
        memcpy((void *)VID_MEM_ADDR, (void *)(VID_MEM_TERM0 + display_term * fourKB), fourKB);
        cursor_update(screen_x[display_term], screen_y[display_term]);
        is_base_shell = 1;
        // create shell process for new terminal
        sys_execute((uint8_t *)"shell \n");
        return 0;
    }
    new_pid = run_queue[(int)new_term_index]->pid;

    page_init(new_pid);
    flush_tlb();

    // set esp0, ss0 in TTS
    tss.esp0 = (uint32_t)(0x800000 - new_pid * 0x2000 - 4);     // end of 8KB block(4 for return address)
    tss.ss0 = KERNEL_DS;

    pcb_t *current_pcb = get_pcb_ptr(current_pid);
    // save ebp, esp of current process
    register uint32_t saved_ebp asm("ebp");
    register uint32_t saved_esp asm("esp");
    current_pcb->saved_ebp = saved_ebp;
    current_pcb->saved_esp = saved_esp;
    current_pid = new_pid;

    display_term = new_term_index;
    running_term = new_term_index;
    vmem_remap();
    // restore new video memory
    memcpy((void *)VID_MEM_ADDR, (void *)(VID_MEM_TERM0 + display_term * fourKB), fourKB);
    cursor_update(screen_x[display_term], screen_y[display_term]);
    current_pcb = get_pcb_ptr(new_pid);
    // context switch
    asm volatile ("                     \n\
            movl %0, %%ebp              \n\
            movl %1, %%esp              \n\
            "
            :                         \
            :  "r"(current_pcb->saved_ebp), "r"(current_pcb->saved_esp)\
            :  "memory", "cc", "eax"
    );
    return 0;
}
